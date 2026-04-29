# Phase 2 — Asset Pipeline & Resource Management — Full Plan

---

## Goal
```
By end of Phase 2:
  ✅ AsyncManager loads files off main thread
  ✅ ResourceManager.Load("player.idle") returns valid asset
  ✅ Same ID loaded 10x → disk read ONCE (cache)
  ✅ AssetTracker.Report() prints status table
  ✅ Zero file paths in game code
  ✅ Main thread never freezes during loads
```

---

## Build Order
```
Step 1  →  AssetEntry + AssetIDs
Step 2  →  AssetDB (full, replaces stub)
Step 3  →  AssetTracker
Step 4  →  AsyncJob + AsyncHandle
Step 5  →  ThreadPool
Step 6  →  MainThreadDispatcher
Step 7  →  AsyncManager
Step 8  →  Loaders (Texture, Font, Sound, Music, Data)
Step 9  →  ResourceManager
Step 10 →  Wire into Application
Step 11 →  Validation
```

---

## Step 1 — AssetEntry + AssetIDs

### AssetEntry
```
STRUCT AssetEntry:
    id              string      "tex.player.idle"
    type            AssetType   TEXTURE | FONT | SOUND | MUSIC | DATA
    realPath        string      "assets/textures/player/idle.png"
    placeholderPath string      "assets/placeholder/player_idle.png"
    status          AssetStatus REAL | PLACEHOLDER | MISSING


ENUM AssetType:
    TEXTURE
    FONT
    SOUND
    MUSIC
    DATA


ENUM AssetStatus:
    REAL            ← real file exists on disk
    PLACEHOLDER     ← only placeholder exists
    MISSING         ← neither exists → needs baking (Phase 6)
```

### AssetIDs
```
// Constants — never type a raw string ID in game code

namespace Assets:
    TEX_UI_BUTTON_NORMAL  = "tex.ui.button.normal"
    TEX_UI_BUTTON_HOVER   = "tex.ui.button.hover"
    TEX_PLAYER_IDLE       = "tex.player.idle"
    FONT_MAIN             = "font.main"
    SFX_JUMP              = "sfx.jump"
    SFX_CLICK             = "sfx.click"
    BGM_MENU              = "bgm.menu"
    TEX_MISSING           = "tex.missing"   ← fallback texture
```

---

## Step 2 — AssetDB (Full, replaces Phase 1 stub)

```
// assets.json now has real entries:
{
  "assets": [
    {
      "id":          "tex.player.idle",
      "type":        "TEXTURE",
      "real":        "assets/textures/player/idle.png",
      "placeholder": "assets/placeholder/player_idle.png"
    },
    {
      "id":          "font.main",
      "type":        "FONT",
      "real":        "assets/fonts/main.ttf",
      "placeholder": ""
    },
    {
      "id":          "sfx.jump",
      "type":        "SOUND",
      "real":        "assets/sounds/sfx/jump.wav",
      "placeholder": "assets/placeholder/sfx_jump.wav"
    },
    ...
  ]
}


CLASS AssetDB:

    entries     map<string, AssetEntry>    ← keyed by id


    FUNCTION Init(json):
        IF json has no "assets" array:
            LOG_WARN("AssetDB: no assets in JSON")
            RETURN

        FOR each item in json["assets"]:
            entry.id              = item["id"]
            entry.type            = parseType(item["type"])
            entry.realPath        = item["real"]
            entry.placeholderPath = item["placeholder"]
            entry.status          = AssetStatus.MISSING  ← default
                                                           AssetTracker
                                                           sets this later

            entries[entry.id] = entry

        LOG_INFO("AssetDB: loaded " + entries.size() + " assets")


    FUNCTION GetEntry(id) → AssetEntry?:
        IF id not in entries:
            LOG_WARN("AssetDB: unknown id: " + id)
            RETURN null
        RETURN entries[id]


    FUNCTION GetAll() → list<AssetEntry>:
        RETURN all values in entries


    FUNCTION GetAllOfType(type) → list<AssetEntry>:
        RETURN entries WHERE entry.type == type


    PRIVATE FUNCTION parseType(string) → AssetType:
        "TEXTURE" → AssetType.TEXTURE
        "FONT"    → AssetType.FONT
        "SOUND"   → AssetType.SOUND
        "MUSIC"   → AssetType.MUSIC
        "DATA"    → AssetType.DATA
        default   → LOG_WARN, return AssetType.DATA
```

---

## Step 3 — AssetTracker

```
CLASS AssetTracker:

    DEPENDS ON: AssetDB


    FUNCTION Init(assetDB):
        m_DB = assetDB
        RescanStatus()              ← check disk on startup
        LOG_INFO("AssetTracker ready")


    FUNCTION RescanStatus():
        // Walk every registered asset, check what exists on disk
        FOR each entry in m_DB.GetAll():
            IF fileExists(entry.realPath):
                entry.status = REAL
            ELSE IF fileExists(entry.placeholderPath):
                entry.status = PLACEHOLDER
            ELSE:
                entry.status = MISSING

        LOG_DEBUG("AssetTracker: rescan complete")


    FUNCTION Resolve(id) → string (path):
        entry = m_DB.GetEntry(id)

        IF entry is null:
            LOG_ERROR("AssetTracker: unknown id: " + id)
            RETURN ""

        IF entry.status == REAL:
            RETURN entry.realPath

        IF entry.status == PLACEHOLDER:
            RETURN entry.placeholderPath

        IF entry.status == MISSING:
            LOG_WARN("AssetTracker: MISSING asset: " + id)
            RETURN ""               ← ResourceManager handles this


    FUNCTION GetStatus(id) → AssetStatus:
        entry = m_DB.GetEntry(id)
        IF entry null: RETURN MISSING
        RETURN entry.status


    FUNCTION GetAllPlaceholders() → list<AssetEntry>:
        RETURN entries WHERE status == PLACEHOLDER


    FUNCTION GetAllMissing() → list<AssetEntry>:
        RETURN entries WHERE status == MISSING


    FUNCTION Report():
        total       = m_DB.GetAll().size()
        realCount   = count WHERE status == REAL
        phCount     = count WHERE status == PLACEHOLDER
        missingCount= count WHERE status == MISSING

        LOG_INFO("╔══════════════════════════════════════╗")
        LOG_INFO("║       ASSET TRACKER REPORT           ║")
        LOG_INFO("╠══════════════════════════════════════╣")
        LOG_INFO("║  Total       : " + total             )
        LOG_INFO("║  ✅ Real      : " + realCount         )
        LOG_INFO("║  🟡 Placeholder: " + phCount          )
        LOG_INFO("║  ❌ Missing   : " + missingCount      )
        LOG_INFO("╠══════════════════════════════════════╣")

        FOR each entry in m_DB.GetAll():
            icon = entry.status == REAL        ? "✅"
                 : entry.status == PLACEHOLDER ? "🟡"
                 : "❌"
            LOG_INFO("║  " + icon + " " + entry.id
                + " (" + typeName(entry.type) + ")")

        LOG_INFO("╚══════════════════════════════════════╝")
```

---

## Step 4 — AsyncJob + AsyncHandle

### AsyncJob
```
STRUCT AsyncJob:
    id          string          UUID.Generate()
    payload     function()→any  the work to run on worker thread
    priority    JobPriority     HIGH | NORMAL | LOW
    status      JobStatus       PENDING | RUNNING | DONE | FAILED
    result      any             filled when DONE
    error       string          filled when FAILED


ENUM JobPriority:
    HIGH    ← scene loading, critical assets
    NORMAL  ← regular asset loads
    LOW     ← background preloads


ENUM JobStatus:
    PENDING
    RUNNING
    DONE
    FAILED
```

### AsyncHandle
```
CLASS AsyncHandle<T>:
    // Caller holds this after Submit()
    // Checks result without blocking

    m_Job       shared_ptr<AsyncJob>


    FUNCTION IsReady() → bool:
        RETURN m_Job.status == DONE


    FUNCTION IsFailed() → bool:
        RETURN m_Job.status == FAILED


    FUNCTION GetResult() → T:
        IF NOT IsReady():
            LOG_WARN("AsyncHandle: result not ready yet")
            RETURN default T
        RETURN cast<T>(m_Job.result)


    FUNCTION OnComplete(callback):
        // callback runs on MAIN THREAD via MainThreadDispatcher
        m_OnComplete = callback


    FUNCTION GetStatus() → JobStatus:
        RETURN m_Job.status
```

---

## Step 5 — ThreadPool

```
CLASS ThreadPool:

    m_Workers       list<Thread>
    m_JobQueue      priority_queue<AsyncJob>   ← sorted by priority
    m_QueueMutex    Mutex
    m_Condition     ConditionVariable
    m_Shutdown      bool = false


    FUNCTION Init(threadCount):
        // default: CPU core count - 1
        // leave 1 core free for main thread
        IF threadCount == 0:
            threadCount = max(1, CPU_CORES - 1)

        FOR i in 0..threadCount:
            thread = Thread( WorkerLoop )
            m_Workers.add(thread)

        LOG_INFO("ThreadPool: started "
            + threadCount + " worker threads")


    FUNCTION Submit(job):
        LOCK m_QueueMutex
            m_JobQueue.push(job)        ← sorted by priority
        UNLOCK
        m_Condition.notify_one()        ← wake a sleeping worker


    FUNCTION Shutdown():
        m_Shutdown = true
        m_Condition.notify_all()        ← wake all workers so they can exit
        FOR each worker:
            worker.join()               ← wait for clean exit
        LOG_INFO("ThreadPool: shutdown complete")


    PRIVATE FUNCTION WorkerLoop():
        // runs on each worker thread forever
        LOOP:
            LOCK m_QueueMutex
                WAIT m_Condition UNTIL (queue not empty OR shutdown)

                IF shutdown AND queue empty:
                    RETURN                  ← clean thread exit

                job = m_JobQueue.top()      ← highest priority first
                m_JobQueue.pop()
            UNLOCK

            // run the job
            job.status = RUNNING
            TRY:
                job.result = job.payload()  ← actual work happens here
                job.status = DONE
            CATCH error:
                job.error  = error.message
                job.status = FAILED
                LOG_ERROR("ThreadPool: job failed: " + error.message)
```

---

## Step 6 — MainThreadDispatcher

```
// Worker threads CANNOT call raylib (OpenGL) functions.
// Texture upload, sound init must happen on main thread.
// MainThreadDispatcher queues callbacks for main thread.

CLASS MainThreadDispatcher:

    m_PendingCallbacks  queue<function()>
    m_Mutex             Mutex


    FUNCTION Queue(callback):
        // called FROM worker thread
        LOCK m_Mutex
            m_PendingCallbacks.push(callback)
        UNLOCK


    FUNCTION Flush():
        // called FROM main thread EVERY FRAME
        // runs all queued callbacks safely
        LOCK m_Mutex
            pending = move(m_PendingCallbacks)  ← drain queue
        UNLOCK

        FOR each callback in pending:
            callback()                           ← runs on main thread


    // Flow:
    // Worker thread: loads raw bytes from disk
    // Worker thread: calls Dispatcher.Queue(uploadToGPU)
    // Main thread:   Dispatcher.Flush() → uploadToGPU runs here
```

---

## Step 7 — AsyncManager

```
CLASS AsyncManager:

    OWNS:
        ThreadPool          m_Pool
        MainThreadDispatcher m_Dispatcher


    FUNCTION Init():
        m_Pool.Init(0)              ← 0 = auto detect core count
        LOG_INFO("AsyncManager ready")


    FUNCTION Submit<T>(payload, priority) → AsyncHandle<T>:
        job = new AsyncJob
        job.id       = UUID.Generate()
        job.payload  = payload
        job.priority = priority
        job.status   = PENDING

        handle = AsyncHandle<T>(job)

        m_Pool.Submit(job)

        RETURN handle


    FUNCTION GetDispatcher() → MainThreadDispatcher&:
        RETURN m_Dispatcher


    FUNCTION Flush():
        // called every frame from Application main loop
        m_Dispatcher.Flush()


    FUNCTION Shutdown():
        m_Pool.Shutdown()


    // Convenience: submit with callback instead of polling handle
    FUNCTION SubmitWithCallback<T>(payload, priority, onDone):
        handle = Submit<T>(payload, priority)

        // poll internally, queue callback when done
        m_Dispatcher.Queue(FUNCTION:
            IF handle.IsReady():
                onDone(handle.GetResult())
        )

        RETURN handle
```

---

## Step 8 — Loaders

> All loaders follow the same pattern:
> - Worker thread: read raw bytes from disk
> - Main thread: upload to GPU / init audio

### TextureLoader
```
CLASS TextureLoader:

    FUNCTION Load(path) → Texture2D:
        // runs on MAIN THREAD (raylib GPU call)
        IF NOT fileExists(path):
            LOG_ERROR("TextureLoader: file not found: " + path)
            RETURN fallbackTexture()

        texture = raylib.LoadTexture(path)
        LOG_DEBUG("TextureLoader: loaded " + path)
        RETURN texture


    FUNCTION Unload(texture):
        raylib.UnloadTexture(texture)


    PRIVATE FUNCTION fallbackTexture() → Texture2D:
        // 8x8 magenta/black checkerboard
        // engine NEVER crashes on missing texture
        image = raylib.GenImageChecked(8, 8, 2, 2, MAGENTA, BLACK)
        RETURN raylib.LoadTextureFromImage(image)
```

### FontLoader
```
CLASS FontLoader:

    FUNCTION Load(path, fontSize) → Font:
        IF NOT fileExists(path):
            LOG_WARN("FontLoader: not found, using default: " + path)
            RETURN raylib.GetFontDefault()

        font = raylib.LoadFontEx(path, fontSize, null, 0)
        LOG_DEBUG("FontLoader: loaded " + path)
        RETURN font


    FUNCTION Unload(font):
        raylib.UnloadFont(font)
```

### SoundLoader
```
CLASS SoundLoader:

    FUNCTION Load(path) → Sound:
        // short SFX — fully loaded into RAM
        IF NOT fileExists(path):
            LOG_WARN("SoundLoader: not found: " + path)
            RETURN emptySound()

        sound = raylib.LoadSound(path)
        LOG_DEBUG("SoundLoader: loaded " + path)
        RETURN sound


    FUNCTION Unload(sound):
        raylib.UnloadSound(sound)
```

### MusicLoader
```
CLASS MusicLoader:

    FUNCTION Load(path) → Music:
        // streamed — does NOT load entire file into RAM
        IF NOT fileExists(path):
            LOG_WARN("MusicLoader: not found: " + path)
            RETURN emptyMusic()

        music = raylib.LoadMusicStream(path)
        LOG_DEBUG("MusicLoader: loaded stream " + path)
        RETURN music


    FUNCTION Unload(music):
        raylib.UnloadMusicStream(music)
```

### DataLoader
```
CLASS DataLoader:

    FUNCTION Load(path) → json:
        RETURN Serializer.LoadFile(path)    ← already safe (returns {} on fail)
```

---

## Step 9 — ResourceManager

```
CLASS ResourceManager:

    DEPENDS ON: AssetTracker, AsyncManager


    // ── Cache (one per asset type) ────────────────────
    m_TextureCache  map<string, Texture2D>
    m_FontCache     map<string, Font>
    m_SoundCache    map<string, Sound>
    m_MusicCache    map<string, Music>
    m_DataCache     map<string, json>

    // ── Loaders ───────────────────────────────────────
    m_TextureLoader TextureLoader
    m_FontLoader    FontLoader
    m_SoundLoader   SoundLoader
    m_MusicLoader   MusicLoader
    m_DataLoader    DataLoader


    // ── Sync Load (blocks main thread — use for small assets) ──

    FUNCTION LoadTexture(id) → Texture2D:
        IF id in m_TextureCache:
            RETURN m_TextureCache[id]           ← cache hit

        path = AssetTracker.Resolve(id)

        IF path is empty:
            LOG_WARN("ResourceManager: missing " + id + " → fallback")
            RETURN m_TextureLoader.fallbackTexture()

        texture = m_TextureLoader.Load(path)
        m_TextureCache[id] = texture
        LOG_DEBUG("ResourceManager: cached texture " + id)
        RETURN texture


    FUNCTION LoadFont(id) → Font:
        IF id in m_FontCache: RETURN m_FontCache[id]
        path = AssetTracker.Resolve(id)
        font = m_FontLoader.Load(path, 32)      ← default size 32
        m_FontCache[id] = font
        RETURN font


    FUNCTION LoadSound(id) → Sound:
        IF id in m_SoundCache: RETURN m_SoundCache[id]
        path = AssetTracker.Resolve(id)
        sound = m_SoundLoader.Load(path)
        m_SoundCache[id] = sound
        RETURN sound


    FUNCTION LoadMusic(id) → Music:
        IF id in m_MusicCache: RETURN m_MusicCache[id]
        path = AssetTracker.Resolve(id)
        music = m_MusicLoader.Load(path)
        m_MusicCache[id] = music
        RETURN music


    FUNCTION LoadData(id) → json:
        IF id in m_DataCache: RETURN m_DataCache[id]
        path = AssetTracker.Resolve(id)
        data = m_DataLoader.Load(path)
        m_DataCache[id] = data
        RETURN data


    // ── Async Load (non-blocking — use for large assets) ──

    FUNCTION LoadTextureAsync(id, onDone):
        IF id in m_TextureCache:
            onDone(m_TextureCache[id])          ← already loaded, instant
            RETURN

        path = AssetTracker.Resolve(id)

        // Phase 1: read raw bytes on worker thread
        AsyncManager.Submit(priority: NORMAL):
            rawBytes = readFileBytes(path)      ← worker thread
            RETURN rawBytes

        // Phase 2: upload to GPU on main thread
        .OnComplete(rawBytes):
            texture = raylib.LoadTextureFromMemory(rawBytes)
            m_TextureCache[id] = texture
            onDone(texture)                     ← main thread callback


    // ── Cache Management ──────────────────────────────

    FUNCTION Unload(id):
        IF id in m_TextureCache:
            m_TextureLoader.Unload(m_TextureCache[id])
            m_TextureCache.remove(id)
        // same for other types...
        LOG_DEBUG("ResourceManager: unloaded " + id)


    FUNCTION UnloadUnused(activeIds: set<string>):
        // called on scene exit — evict anything not in active set
        FOR each id in m_TextureCache:
            IF id NOT in activeIds:
                Unload(id)
        LOG_INFO("ResourceManager: unloaded unused assets")


    FUNCTION Clear():
        // full wipe — called on engine shutdown
        FOR each texture in m_TextureCache:
            m_TextureLoader.Unload(texture)
        FOR each font in m_FontCache:
            m_FontLoader.Unload(font)
        FOR each sound in m_SoundCache:
            m_SoundLoader.Unload(sound)
        FOR each music in m_MusicCache:
            m_MusicLoader.Unload(music)

        m_TextureCache.clear()
        m_FontCache.clear()
        m_SoundCache.clear()
        m_MusicCache.clear()
        m_DataCache.clear()

        LOG_INFO("ResourceManager: all assets cleared")


    FUNCTION GetCacheStats():
        LOG_INFO("ResourceManager Cache:")
        LOG_INFO("  Textures : " + m_TextureCache.size())
        LOG_INFO("  Fonts    : " + m_FontCache.size())
        LOG_INFO("  Sounds   : " + m_SoundCache.size())
        LOG_INFO("  Music    : " + m_MusicCache.size())
        LOG_INFO("  Data     : " + m_DataCache.size())
```

---

## Step 10 — Wire Into Application

```
CLASS Application:

    OWNS (new in Phase 2):
        AssetTracker        m_AssetTracker
        AsyncManager        m_Async
        ResourceManager     m_Resources


    FUNCTION Init():
        Logger.Init()
        LOG_INFO("=== Zhenzhu Engine ===")

        // Phase 1
        m_Data.Init()

        // Window from settings
        m_Window.Create(m_Data.settings)

        // Phase 2 — boot order matters
        m_Async.Init()                          ← threads start
        m_AssetTracker.Init(m_Data.assets)      ← scans disk
        m_AssetTracker.RescanStatus()           ← REAL/PLACEHOLDER/MISSING
        m_AssetTracker.Report()                 ← print table

        m_Resources.Init(m_AssetTracker, m_Async)

        // Register all services
        ServiceLocator.Register(m_Data)
        ServiceLocator.Register(m_Async)
        ServiceLocator.Register(m_AssetTracker)
        ServiceLocator.Register(m_Resources)

        LOG_INFO("Application initialized ✓")


    FUNCTION Run():
        WHILE running AND NOT window.ShouldClose():
            m_Timer.Tick()
            dt = m_Timer.GetDeltaTime()

            // Phase 2 — flush async callbacks every frame
            m_Async.Flush()                     ← NEW

            ProcessInput()
            FixedUpdate()
            Update(dt)
            Render()


    FUNCTION Shutdown():
        m_Resources.Clear()                     ← unload GPU assets first
        m_Async.Shutdown()                      ← drain threads
        m_Window.Close()
        Logger.Shutdown()
```

---

## Step 11 — Update SConstruct

```python
# Phase 2 adds .cpp files → must add to SConstruct

ENGINE_SOURCES = [
    # Phase 0
    "engine/core/Application.cpp",
    "engine/core/Window.cpp",
    "engine/core/Timer.cpp",
    "engine/utils/Logger.cpp",

    # Phase 2 — new .cpp files
    "engine/async/ThreadPool.cpp",
    "engine/async/AsyncManager.cpp",
    "engine/async/MainThreadDispatcher.cpp",
    "engine/resources/ResourceManager.cpp",
    "engine/resources/TextureLoader.cpp",
    "engine/resources/FontLoader.cpp",
    "engine/resources/SoundLoader.cpp",
    "engine/resources/MusicLoader.cpp",
    "engine/resources/DataLoader.cpp",
    "engine/assets/AssetTracker.cpp",
]

# All data/, utils/, assets/ headers are header-only
# No .cpp needed for: AssetEntry, AssetIDs, AssetDB,
#                     Serializer, EventBus, Math2D, UUID
```

---

## Step 12 — Async Load Flow (Full Picture)

```
// How a texture gets from disk to GPU safely

SYNC path (small assets, startup):
──────────────────────────────────
  ResourceManager.LoadTexture("tex.player.idle")
      │
      ├── cache miss
      ├── AssetTracker.Resolve("tex.player.idle")
      │       └── returns "assets/placeholder/player_idle.png"
      ├── TextureLoader.Load(path)
      │       └── raylib.LoadTexture()   ← main thread, fine
      ├── store in m_TextureCache
      └── return Texture2D


ASYNC path (large assets, scene loads):
──────────────────────────────────────
  ResourceManager.LoadTextureAsync("tex.bigmap", onDone)
      │
      ├── cache miss
      ├── AssetTracker.Resolve() → path
      │
      ├── AsyncManager.Submit():           ← returns immediately
      │       Worker Thread:
      │           rawBytes = readFile(path) ← disk I/O off main thread
      │           job.status = DONE
      │
      ├── OnComplete → MainThreadDispatcher.Queue():
      │       Main Thread (next Flush()):
      │           texture = LoadTextureFromMemory(rawBytes)
      │           m_TextureCache[id] = texture
      │           onDone(texture)          ← caller gets texture here
      │
      └── main thread NEVER blocked during disk read
```

---

## Step 13 — Validation Tests

```
TEST 1 — AssetDB loads entries:
    Run engine
    Expected log:
        "AssetDB: loaded 8 assets"
        "AssetTracker: rescan complete"

TEST 2 — AssetTracker.Report() prints table:
    Expected log:
        ╔══════════════════════════════════════╗
        ║       ASSET TRACKER REPORT           ║
        ╠══════════════════════════════════════╣
        ║  Total        : 8                    ║
        ║  ✅ Real       : 0                   ║
        ║  🟡 Placeholder: 0                   ║
        ║  ❌ Missing    : 8                   ║
        ╚══════════════════════════════════════╝
    (all missing because we have no real/placeholder files yet)
    No crash.

TEST 3 — ResourceManager fallback on missing:
    texture = ResourceManager.LoadTexture("tex.player.idle")
    Expected:
        LOG_WARN: "AssetTracker: MISSING asset: tex.player.idle"
        LOG_WARN: "ResourceManager: missing → fallback"
        Returns magenta/black checkerboard texture
        No crash

TEST 4 — ResourceManager cache:
    t1 = ResourceManager.LoadTexture("tex.missing")
    t2 = ResourceManager.LoadTexture("tex.missing")
    Expected:
        First call: LOG_DEBUG "cached texture tex.missing"
        Second call: NO log (cache hit, no disk read)
        t1 == t2 (same object)

TEST 5 — AsyncManager submits job:
    handle = AsyncManager.Submit(
        payload: FUNCTION: RETURN 42
        priority: NORMAL
    )
    Wait 1 frame
    Expected:
        handle.IsReady() == true
        handle.GetResult() == 42

TEST 6 — MainThreadDispatcher flushes:
    flag = false
    Dispatcher.Queue(FUNCTION: flag = true)
    Before Flush(): flag == false
    After  Flush(): flag == true

TEST 7 — ThreadPool spins correct threads:
    Expected log:
        "ThreadPool: started N worker threads"
        where N == CPU_CORES - 1

TEST 8 — GetCacheStats:
    Call ResourceManager.GetCacheStats()
    Expected log:
        ResourceManager Cache:
          Textures : 1
          Fonts    : 0
          Sounds   : 0
          Music    : 0
          Data     : 0

TEST 9 — Shutdown clean:
    Close engine
    Expected log:
        "ResourceManager: all assets cleared"
        "ThreadPool: shutdown complete"
    No hanging threads, no crashes
```

---

## Phase 2 Checklist

```
CONFIG:
  ✅ assets.json populated with real entries
  ✅ assets/ folder structure created

ASSET SYSTEM:
  ✅ AssetEntry       (id, type, realPath, placeholderPath, status)
  ✅ AssetIDs.hpp     (constants for all asset IDs)
  ✅ AssetDB          (full Init, GetEntry, GetAll, GetAllOfType)
  ✅ AssetTracker     (Init, Resolve, GetStatus, RescanStatus, Report)

ASYNC:
  ✅ AsyncJob         (payload, priority, status, result)
  ✅ AsyncHandle<T>   (IsReady, GetResult, OnComplete)
  ✅ ThreadPool       (Init, Submit, WorkerLoop, Shutdown)
  ✅ MainThreadDispatcher (Queue, Flush)
  ✅ AsyncManager     (Init, Submit, Flush, Shutdown)

LOADERS:
  ✅ TextureLoader    (Load, Unload, fallbackTexture)
  ✅ FontLoader       (Load, Unload, default font fallback)
  ✅ SoundLoader      (Load, Unload, empty sound fallback)
  ✅ MusicLoader      (Load, Unload, empty music fallback)
  ✅ DataLoader       (Load via Serializer)

RESOURCE MANAGER:
  ✅ LoadTexture      (sync, cached)
  ✅ LoadFont         (sync, cached)
  ✅ LoadSound        (sync, cached)
  ✅ LoadMusic        (sync, cached)
  ✅ LoadData         (sync, cached)
  ✅ LoadTextureAsync (async, callback)
  ✅ Unload           (single asset)
  ✅ UnloadUnused     (scene exit eviction)
  ✅ Clear            (full shutdown wipe)
  ✅ GetCacheStats    (debug info)

WIRED:
  ✅ Application boots AsyncManager before ResourceManager
  ✅ Application boots AssetTracker before ResourceManager
  ✅ Application.Run() calls AsyncManager.Flush() every frame
  ✅ Application.Shutdown() calls Resources.Clear() then Async.Shutdown()
  ✅ ServiceLocator has AsyncManager, AssetTracker, ResourceManager

VALIDATED:
  ✅ All 9 tests pass
  ✅ No crash on missing assets (fallback texture shows)
  ✅ Cache hit confirmed (no double disk reads)
  ✅ No thread leaks on shutdown
  ✅ AssetTracker.Report() prints correct status table
```

---

## What Phase 3 Gets For Free

```
Phase 3 (Renderer & Input) immediately has:
  ✅ ResourceManager.LoadTexture(id)   → draw sprites
  ✅ ResourceManager.LoadFont(id)      → draw text
  ✅ AssetTracker status               → debug overlay
  ✅ AsyncManager                      → preload next scene
  ✅ Fallback texture                  → never crash on missing art
  ✅ Cache                             → same texture drawn 1000x,
                                         loaded from GPU once
```

Ready to move to Phase 3 plan!