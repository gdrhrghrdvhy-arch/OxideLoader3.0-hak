#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <atomic>
#include <vector>

#define LOG_TAG "OxideCheat"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef uint64_t u64;
typedef uint32_t u32;
typedef float f32;

struct Settings {
    std::atomic<bool> aimbot{true};
    std::atomic<bool> aim_head{true};
    std::atomic<bool> esp{true};
    std::atomic<bool> wallhack{true};
    std::atomic<bool> anti_ban{true};
};
Settings g_settings;

struct Offsets {
    u64 gworld = 0x693f904;
    u64 ulevel = 0x30;
    u64 actors = 0x20;
    u64 actor_count = 0x28;
    u64 local_player = 0x28;
    u64 player_controller = 0x38;
    u64 pawn = 0x4A8;
    u64 health = 0x1A0;
    u64 team_id = 0x1F4;
    u64 position = 0x150;
    u64 root_component = 0x1D0;
    u64 last_render_time = 0x360;
    u64 camera_manager = 0x458;
    u64 camera_cache = 0x1B80;
};
Offsets off;

int mem_fd = -1;
bool init_mem() {
    mem_fd = open("/proc/self/mem", O_RDWR);
    return mem_fd >= 0;
}
template<typename T> T read_mem(u64 addr) {
    T v = T(0);
    if (mem_fd < 0 || !addr) return v;
    pread(mem_fd, &v, sizeof(T), (off_t)addr);
    return v;
}
template<typename T> void write_mem(u64 addr, T v) {
    if (mem_fd < 0 || !addr) return;
    pwrite(mem_fd, &v, sizeof(T), (off_t)addr);
}

u64 get_base(const char* name = "libUE4.so") {
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return 0;
    char line[1024]; u64 b = 0;
    while (fgets(line, sizeof(line), f))
        if (strstr(line, name)) { b = strtoull(line, nullptr, 16); break; }
    fclose(f); return b;
}

u64 get_local_pawn(u64 gworld) {
    u64 gi = read_mem<u64>(gworld + 0x38);
    if (!gi) return 0;
    u64 lp = read_mem<u64>(gi + off.local_player);
    u64 pc = read_mem<u64>(lp + off.player_controller);
    return read_mem<u64>(pc + off.pawn);
}

void do_cheat_logic(u64 gworld, u64 local_pawn) {
    u64 ulevel = read_mem<u64>(gworld + off.ulevel);
    if (!ulevel) return;
    u64 actors = read_mem<u64>(ulevel + off.actors);
    u32 count = read_mem<u32>(ulevel + off.actor_count);
    u32 my_team = read_mem<u32>(local_pawn + off.team_id);

    u64 cam = read_mem<u64>(local_pawn + off.camera_manager);
    u64 pov = cam + off.camera_cache;
    f32 cx = read_mem<f32>(pov), cy = read_mem<f32>(pov + 4), cz = read_mem<f32>(pov + 8);
    f32 cur_pitch = read_mem<f32>(pov + 0x30);
    f32 cur_yaw = read_mem<f32>(pov + 0x34);

    u64 best_target = 0;
    f32 best_dist = 9999.0f;
    f32 tgt_x = 0, tgt_y = 0, tgt_z = 0;

    for (u32 i = 0; i < count && i < 1000; i++) {
        u64 a = read_mem<u64>(actors + i * 8);
        if (a == local_pawn || !a) continue;
        if (read_mem<u32>(a + off.team_id) == my_team) continue;
        
        f32 hp = read_mem<f32>(a + off.health);
        if (hp <= 0 || hp > 1000) continue;

        if (g_settings.wallhack) write_mem<f32>(a + off.last_render_time, 999.0f);

        u64 root = read_mem<u64>(a + off.root_component);
        if (!root) continue;
        f32 x = read_mem<f32>(root + off.position);
        f32 y = read_mem<f32>(root + off.position + 4);
        f32 z = read_mem<f32>(root + off.position + 8);

        if (g_settings.aimbot) {
            if (g_settings.aim_head) z += 60.0f;
            f32 dx = x - cx, dy = y - cy, dz = z - cz;
            f32 dist = sqrtf(dx*dx + dy*dy + dz*dz);
            if (dist < best_dist) {
                best_dist = dist;
                best_target = a;
                tgt_x = x; tgt_y = y; tgt_z = z;
            }
        }
    }

    if (g_settings.aimbot && best_target) {
        f32 dx = tgt_x - cx, dy = tgt_y - cy, dz = tgt_z - cz;
        f32 dist = sqrtf(dx*dx + dy*dy + dz*dz);
        f32 pitch = asinf(dz / dist) * 57.2958f;
        f32 yaw = atan2f(dy, dx) * 57.2958f;
        if (fabsf(pitch - cur_pitch) < 30.0f && fabsf(yaw - cur_yaw) < 30.0f) {
            write_mem<f32>(pov + 0x30, pitch);
            write_mem<f32>(pov + 0x34, yaw);
        }
    }
}

void* cheat_thread(void*) {
    if (!init_mem()) return nullptr;
    u64 base = get_base();
    if (!base) return nullptr;
    while (true) {
        u64 gworld = read_mem<u64>(base + off.gworld);
        if (gworld) {
            u64 pawn = get_local_pawn(gworld);
            if (pawn) do_cheat_logic(gworld, pawn);
        }
        usleep(16000);
    }
    return nullptr;
}

__attribute__((constructor)) static void init() {
    pthread_t t;
    pthread_create(&t, nullptr, cheat_thread, nullptr);
    pthread_detach(t);
}
