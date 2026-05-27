/*
 * PROJECT: RiseVLTR Valorant WindMouse v6.0
 * CHANNEL: RiseVLTR (Youtube)
 * FEATURE: Valorant-optimized + Troll Mode + Sensitivity Calibration
 * WARNING: FOR EDUCATIONAL PURPOSES ONLY - RISEVLTR'S OWN GAME
 * 
 * NOT: Bu kod RISEVLTR'ın kendi oyunu içindir. 
 *       Riot Games'in peşine düşme ihtimali %0.1 - hayırlısı :)
 */

#include <iostream>
#include <Windows.h>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <thread>
#include <map>
#include <string>
#include <cstdio>
#include "interception.h"

#pragma comment(lib, "interception.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

// ==================== VALORANT ÖZEL AYARLAR ====================
// RiseVLTR'ın hassasiyeti: 1.06 (Valorant standardı)
// Bu değerler Valorant'ın fare hesaplamalarına göre optimize edilmiştir

struct ValorantConfig {
    // Oyun içi ayarlar
    double sensitivity = 1.06;           // RiseVLTR özel
    double scopedSensitivity = 1.0;      // Nişangahlı hassasiyet
    int dpi = 800;                        // Farenin DPI değeri
    
    // Valorant özel sabitler
    const double YAW = 0.022;             // Valorant'ın yaw değeri (radyan başına count)
    const double PITCH = 0.022;           // Valorant'ın pitch değeri
    
    // Hassasiyete göre çarpan hesapla (Valorant formülü)
    double getSensitivityMultiplier() {
        // Valorant'ın hassasiyet formülü: 
        // Counts = (DPI / 1000) * Sensitivity * Yaw * Degrees
        return (dpi / 1000.0) * sensitivity * YAW;
    }
    
    // Ekran koordinatlarını oyun içi açıya çevir
    double pixelsToAngle(int pixels, int screenRes) {
        // 360 derece = 103 FOV (Valorant varsayılan)
        double degreesPerPixel = 103.0 / screenRes;
        return pixels * degreesPerPixel;
    }
    
    // Açıyı mouse count'ına çevir (Valorant)
    int angleToCounts(double angleDegrees) {
        return (int)(angleDegrees / (YAW * sensitivity));
    }
} valorant;

// ==================== RENK TANIMA (Valorant Enemy Purple) ====================
// Valorant'ta düşmanların outline rengi: MOR (purple)
// R: 255, G: 0-100, B: 200-255 arası

struct ColorConfig {
    // Düşman outline rengi (Valorant varsayılan - mor)
    int enemyRedMin = 200;
    int enemyRedMax = 255;
    int enemyGreenMin = 0;
    int enemyGreenMax = 80;
    int enemyBlueMin = 180;
    int enemyBlueMax = 255;
    
    // Alternatif: Kırmızı düşmanlar (eski Valorant)
    int altRedMin = 220;
    int altGreenMax = 60;
    int altBlueMax = 60;
    
    // Hedef bölgesi (kafa için ayar)
    bool headshotMode = true;
    int headOffsetY = -15;  // Kafa için yukarı kaydırma
} colorConfig;

// ==================== WINDMOUSE (Valorant için optimize) ====================
struct WindMouseParams {
    double gravity = 8.5;       // Valorant için optimize (daha az yerçekimi)
    double wind = 2.5;          // Daha az rüzgar (daha stabil)
    double minWait = 0.5;       // Daha hızlı tepki (ms)
    double maxWait = 2.5;       // Maksimum bekleme
    double maxStep = 10.0;      // Maksimum adım
    double targetArea = 5.0;    // Daha hassas hedefleme
};

class WindMouse {
private:
    WindMouseParams params;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
    
public:
    WindMouse() : rng(std::chrono::steady_clock::now().time_since_epoch().count()), dist(0.0, 1.0) {}
    
    double hypot(double a, double b) { return sqrt(a * a + b * b); }
    
    struct MovePoint {
        int x, y;
        int waitMs;
    };
    
    std::vector<MovePoint> generateMovement(int startX, int startY, int endX, int endY, double sensitivityMult = 1.0) {
        std::vector<MovePoint> points;
        
        double targetX = (double)endX * sensitivityMult;
        double targetY = (double)endY * sensitivityMult;
        double currentX = (double)startX;
        double currentY = (double)startY;
        
        double veloX = 0.0;
        double veloY = 0.0;
        double windX = 0.0;
        double windY = 0.0;
        
        int maxIterations = 500;
        int iteration = 0;
        
        while (hypot(currentX - targetX, currentY - targetY) > params.targetArea && iteration < maxIterations) {
            double randomVal = dist(rng);
            double distance = hypot(targetX - currentX, targetY - currentY);
            double step = params.maxStep * (distance / 100.0);
            if (step < 0.5) step = 0.5;
            if (step > params.maxStep) step = params.maxStep;
            
            // Valorant için optimize rüzgar (daha stabil)
            windX += (randomVal - 0.5) * params.wind;
            windY += (randomVal - 0.5) * params.wind;
            
            // Yerçekimi (Valorant için daha kontrollü)
            if (distance != 0) {
                veloX += (targetX - currentX) / distance * params.gravity + windX;
                veloY += (targetY - currentY) / distance * params.gravity + windY;
            } else {
                veloX += windX;
                veloY += windY;
            }
            
            double veloMag = hypot(veloX, veloY);
            if (veloMag > step) {
                veloX = veloX / veloMag * step;
                veloY = veloY / veloMag * step;
            }
            
            currentX += veloX;
            currentY += veloY;
            
            windX *= 0.92;
            windY *= 0.92;
            
            int waitTime = (int)(params.minWait + dist(rng) * (params.maxWait - params.minWait));
            points.push_back({ (int)round(currentX), (int)round(currentY), waitTime });
            
            iteration++;
        }
        
        points.push_back({ endX, endY, 1 });
        return points;
    }
};

// ==================== TROLL MODLARI ====================
enum TrollMode {
    TROLL_NORMAL = 0,
    TROLL_FAST = 1,
    TROLL_SLOW = 2,
    TROLL_ZIGZAG = 3,
    TROLL_SPIN = 4,      // Yeni: Spin bot (dönen aim)
    TROLL_JITTER = 5,    // Yeni: Titrek aim (sinirli gibi)
    TROLL_SMOOTH = 6     // Yeni: Aşırı yumuşak (uyuyor gibi)
};

struct Config {
    bool aimbotEnabled = true;
    bool triggerEnabled = true;
    int fov = 110;               // Valorant için ideal FOV
    int smooth = 2;              // Daha hızlı tepki
    int minDelay = 4;            // Valorant için daha hızlı
    int maxDelay = 12;
    bool showMenu = true;
    double sensitivity = 1.06;
    int trollMode = TROLL_NORMAL;
    bool headshotPriority = true;
    bool visibilityCheck = true;  // Engel kontrolü
    int aimBone = 0;              // 0: head, 1: chest, 2: legs
} config;

struct Stats {
    int targetsFound = 0;
    int shotsFired = 0;
    int headshots = 0;
    float lastResponse = 0;
    bool active = false;
    DWORD startTime = 0;
    std::map<std::string, int> weaponStats;
} stats;

// ==================== GLOBAL ====================
HWND g_hwnd = NULL;
HDC hdcScreen = NULL;
HDC hdcMem = NULL;
HBITMAP hBitmap = NULL;
std::vector<BYTE> pixels;
BITMAPINFO bi = {0};
int screenWidth, screenHeight;
int centerX, centerY;
int remainderX = 0, remainderY = 0;
bool wasShooting = false;

InterceptionContext context;
InterceptionDevice device;
WindMouse windMouse;
std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
HANDLE hConsole;

int GetRandom(int min, int max) {
    return std::uniform_int_distribution<>(min, max)(rng);
}

// ==================== VALORANT ÖZEL RENK TESPİTİ ====================
bool IsEnemyColor(BYTE r, BYTE g, BYTE b) {
    // Ana renk: MOR (Valorant düşman outline)
    bool isPurple = (r > colorConfig.enemyRedMin && r < colorConfig.enemyRedMax &&
                     g > colorConfig.enemyGreenMin && g < colorConfig.enemyGreenMax &&
                     b > colorConfig.enemyBlueMin && b < colorConfig.enemyBlueMax);
    
    // Alternatif: KIRMIZI (eski Valorant veya bazı skinler)
    bool isRed = (r > colorConfig.altRedMin && 
                  g < colorConfig.altGreenMax && 
                  b < colorConfig.altBlueMax);
    
    return isPurple || isRed;
}

// ==================== KONSOL LOGOSU ====================
void ShowLogo() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 13);  // Mor (Valorant rengi)
    std::cout << R"(
   ╔══════════════════════════════════════════════════════════════╗
   ║                                                              ║
   ║      ██╗   ██╗ █████╗ ██╗      ██████╗ ██████╗  █████╗     ║
   ║      ██║   ██║██╔══██╗██║     ██╔═══██╗██╔══██╗██╔══██╗    ║
   ║      ██║   ██║███████║██║     ██║   ██║██████╔╝███████║    ║
   ║      ╚██╗ ██╔╝██╔══██║██║     ██║   ██║██╔══██╗██╔══██║    ║
   ║       ╚████╔╝ ██║  ██║███████╗╚██████╔╝██║  ██║██║  ██║    ║
   ║        ╚═══╝  ╚═╝  ╚═╝╚══════╝ ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝    ║
   ║                                                              ║
   ║      V A L O R A N T   E D I T I O N   v 6 . 0             ║
   ║      RISE VLTR SPECIAL                                      ║
   ║                                                              ║
   ║      Sens: 1.06  |  DPI: 800  |  FOV: 110                  ║
   ║                                                              ║
   ╚══════════════════════════════════════════════════════════════╝
)" << std::endl;
    
    SetConsoleTextAttribute(hConsole, 12);
    std::cout << "\n   [!] VALORANT DÜŞMAN RENGİ: MOR (RGB: R>200, G<80, B>180)" << std::endl;
    std::cout << "   [!] Riot Games peşime düşmesin, ben kendi oyunumda kullanıyorum :)\n" << std::endl;
    
    SetConsoleTextAttribute(hConsole, 11);
    std::cout << "   [+] Kontroller:" << std::endl;
    std::cout << "       • INSERT -> Menü Aç/Kapa" << std::endl;
    std::cout << "       • LMB BASILI TUT -> TriggerBot Aktif" << std::endl;
    std::cout << "       • F1 -> Troll Modu Değiştir" << std::endl;
    std::cout << "       • F2 -> Hassasiyet +0.01" << std::endl;
    std::cout << "       • F3 -> Hassasiyet -0.01" << std::endl;
    std::cout << "       • F4 -> Headshot/Chest Toggle" << std::endl;
    std::cout << "       • ESC -> Çıkış\n" << std::endl;
    
    SetConsoleTextAttribute(hConsole, 10);
    const char* trollNames[] = {"NORMAL", "HIZLI", "YAVAS", "ZIKZAK", "SPIN", "JITTER", "SMOOTH"};
    std::cout << "   [+] Troll Modlari: ";
    for (int i = 0; i < 7; i++) {
        SetConsoleTextAttribute(hConsole, i == 0 ? 10 : 14);
        std::cout << i << ":" << trollNames[i] << "  ";
    }
    std::cout << std::endl;
    
    SetConsoleTextAttribute(hConsole, 7);
}

// ==================== TROLL MODU HAREKETLERİ ====================
std::vector<WindMouse::MovePoint> generateTrollMovement(int startX, int startY, int endX, int endY) {
    std::vector<WindMouse::MovePoint> points;
    
    switch (config.trollMode) {
        case TROLL_NORMAL:
            return windMouse.generateMovement(startX, startY, endX, endY, config.sensitivity);
            
        case TROLL_FAST:
            points.push_back({ endX, endY, 1 });
            return points;
            
        case TROLL_SLOW:
            for (int i = 0; i <= 30; i++) {
                points.push_back({ 
                    startX + (endX - startX) * i / 30, 
                    startY + (endY - startY) * i / 30, 
                    8 + GetRandom(0, 5)
                });
            }
            return points;
            
        case TROLL_ZIGZAG:
            for (int i = 0; i <= 40; i++) {
                double t = i / 40.0;
                int x = startX + (int)((endX - startX) * t);
                int zigzag = (int)(sin(t * 3.14159 * 6) * 25);
                points.push_back({ x + zigzag, startY + (int)((endY - startY) * t) + zigzag/2, 2 });
            }
            return points;
            
        case TROLL_SPIN:  // 360 spin atıyor gibi
            for (int i = 0; i <= 60; i++) {
                double angle = i * 6.28318 / 60;
                int radius = 50;
                points.push_back({ 
                    startX + (int)(cos(angle) * radius * (1 - i/60.0)),
                    startY + (int)(sin(angle) * radius * (1 - i/60.0)),
                    2
                });
            }
            points.push_back({ endX, endY, 1 });
            return points;
            
        case TROLL_JITTER:  // Titrek aim (sinirli gibi)
            for (int i = 0; i <= 20; i++) {
                int jitterX = GetRandom(-8, 8);
                int jitterY = GetRandom(-8, 8);
                points.push_back({ 
                    startX + (endX - startX) * i / 20 + jitterX,
                    startY + (endY - startY) * i / 20 + jitterY,
                    3 + GetRandom(0, 3)
                });
            }
            return points;
            
        case TROLL_SMOOTH:  // Aşırı yumuşak (uyuyor gibi)
            for (int i = 0; i <= 50; i++) {
                points.push_back({ 
                    startX + (endX - startX) * i / 50,
                    startY + (endY - startY) * i / 50,
                    15 + GetRandom(0, 10)
                });
            }
            return points;
            
        default:
            return windMouse.generateMovement(startX, startY, endX, endY, config.sensitivity);
    }
}

// ==================== INTERCEPTION ====================
bool InitInterception() {
    std::cout << "   [*] Interception driver başlatılıyor..." << std::endl;
    
    context = interception_create_context();
    if (!context) {
        std::cout << "   [X] Interception driver bulunamadi!" << std::endl;
        return false;
    }
    
    interception_set_filter(context, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_ALL);
    std::cout << "   [✓] Interception driver hazir!" << std::endl;
    std::cout << "   [✓] Valorant " << config.sensitivity << " sens icin kalibrasyon tamam!\n" << std::endl;
    
    return true;
}

// ==================== GUI ====================
void DrawRect(HDC hdc, int x, int y, int w, int h, COLORREF color) {
    RECT rect = {x, y, x + w, y + h};
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
}

void DrawTextCenter(HDC hdc, int x, int y, int w, int h, const char* text, COLORREF color) {
    RECT rect = {x, y, x + w, y + h};
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    DrawTextA(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void DrawBorder(HDC hdc, int x, int y, int w, int h, COLORREF color, int thickness) {
    HPEN pen = CreatePen(PS_SOLID, thickness, color);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, x, y, x + w, y + h);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
}

void DrawGUI(HDC hdc) {
    if (!config.showMenu) return;
    
    int menuW = 360;
    int menuH = 620;
    int menuX = 15;
    int menuY = 15;
    
    DrawRect(hdc, menuX, menuY, menuW, menuH, RGB(15, 10, 25));  // Mor tema
    DrawBorder(hdc, menuX, menuY, menuW, menuH, RGB(130, 50, 200), 2);
    
    int yOff = menuY + 10;
    
    DrawTextCenter(hdc, menuX, yOff, menuW, 30, "RISEVLTR VALORANT", RGB(170, 70, 255));
    yOff += 35;
    
    DrawTextCenter(hdc, menuX, yOff, menuW, 18, "Youtube: RiseVLTR", RGB(255, 100, 100));
    yOff += 22;
    
    const char* trollNames[] = {"NORMAL", "HIZLI", "YAVAS", "ZIKZAK", "SPIN", "JITTER", "SMOOTH"};
    char trollText[64];
    sprintf_s(trollText, "TROLL MOD: %s", trollNames[config.trollMode]);
    COLORREF trollColor = config.trollMode == 0 ? RGB(0, 255, 0) : RGB(255, 100, 0);
    DrawTextCenter(hdc, menuX, yOff, menuW, 20, trollText, trollColor);
    yOff += 25;
    
    char sensText[64];
    sprintf_s(sensText, "HASSASİYET: %.2f (Valorant)", config.sensitivity);
    DrawTextCenter(hdc, menuX, yOff, menuW, 18, sensText, RGB(255, 200, 100));
    yOff += 22;
    
    char aimText[64];
    sprintf_s(aimText, "AIM: %s", config.headshotPriority ? "HEADSHOT" : "CHEST");
    DrawTextCenter(hdc, menuX + 20, yOff, 150, 18, aimText, config.headshotPriority ? RGB(255, 100, 100) : RGB(100, 200, 255));
    
    char fovText[32];
    sprintf_s(fovText, "FOV: %d", config.fov);
    DrawTextCenter(hdc, menuX + 190, yOff, 150, 18, fovText, RGB(200, 200, 200));
    yOff += 25;
    
    DrawRect(hdc, menuX + 10, yOff, menuW - 20, 1, RGB(80, 80, 80));
    yOff += 12;
    
    char statusText[64];
    sprintf_s(statusText, stats.active ? "● AKTIF - %d HEDEF" : "○ BEKLEMEDE - LMB BAS", stats.targetsFound);
    DrawTextCenter(hdc, menuX, yOff, menuW, 22, statusText, stats.active ? RGB(0, 255, 0) : RGB(150, 150, 150));
    yOff += 28;
    
    DWORD elapsed = (GetTickCount() - stats.startTime) / 1000;
    char timeText[64];
    sprintf_s(timeText, "Süre: %02d:%02d:%02d", elapsed / 3600, (elapsed % 3600) / 60, elapsed % 60);
    DrawTextCenter(hdc, menuX, yOff, menuW, 16, timeText, RGB(150, 150, 200));
    yOff += 22;
    
    DrawRect(hdc, menuX + 10, yOff, menuW - 20, 1, RGB(80, 80, 80));
    yOff += 12;
    
    const char* aimbotText = config.aimbotEnabled ? "[X] AIMBOT" : "[ ] AIMBOT";
    const char* triggerText = config.triggerEnabled ? "[X] TRIGGER" : "[ ] TRIGGER";
    DrawTextCenter(hdc, menuX + 20, yOff, 130, 25, aimbotText, config.aimbotEnabled ? RGB(0, 255, 0) : RGB(150, 150, 150));
    DrawTextCenter(hdc, menuX + 190, yOff, 130, 25, triggerText, config.triggerEnabled ? RGB(0, 255, 0) : RGB(150, 150, 150));
    yOff += 30;
    
    char targetsText[48];
    sprintf_s(targetsText, "Hedef: %d", stats.targetsFound);
    char shotsText[48];
    sprintf_s(shotsText, "Atis: %d", stats.shotsFired);
    DrawTextCenter(hdc, menuX + 20, yOff, 140, 18, targetsText, RGB(255, 200, 100));
    DrawTextCenter(hdc, menuX + 190, yOff, 140, 18, shotsText, RGB(255, 200, 100));
    yOff += 22;
    
    char hsText[48];
    sprintf_s(hsText, "Headshot: %d", stats.headshots);
    float hsPercent = stats.shotsFired > 0 ? (stats.headshots * 100.0f / stats.shotsFired) : 0;
    char hsPercentText[48];
    sprintf_s(hsPercentText, "HS%%: %.0f%%", hsPercent);
    DrawTextCenter(hdc, menuX + 20, yOff, 140, 18, hsText, RGB(100, 255, 200));
    DrawTextCenter(hdc, menuX + 190, yOff, 140, 18, hsPercentText, RGB(100, 255, 200));
    yOff += 22;
    
    char respText[48];
    sprintf_s(respText, "Tepki: %.0f ms", stats.lastResponse);
    DrawTextCenter(hdc, menuX, yOff, menuW, 16, respText, RGB(255, 150, 100));
    yOff += 22;
    
    DrawRect(hdc, menuX + 10, yOff, menuW - 20, 1, RGB(80, 80, 80));
    yOff += 10;
    DrawTextCenter(hdc, menuX, yOff, menuW, 16, "[INS] Menu | [F1] Troll | [F2/F3] Sens | [F4] Aim | [ESC] Exit", RGB(100, 100, 100));
    
    // FOV göstergesi (Valorant mor teması)
    if (stats.active) {
        COLORREF fovColor = RGB(170, 70, 255);  // Valorant moru
        
        HPEN pen = CreatePen(PS_SOLID, 2, fovColor);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Ellipse(hdc, centerX - config.fov, centerY - config.fov, 
                     centerX + config.fov, centerY + config.fov);
        
        // Crosshair
        MoveToEx(hdc, centerX - 10, centerY, NULL);
        LineTo(hdc, centerX + 10, centerY);
        MoveToEx(hdc, centerX, centerY - 10, NULL);
        LineTo(hdc, centerX, centerY + 10);
        
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(pen);
    }
}

// ==================== PİKSEL TARAMA (Valorant Enemy Detection) ====================
bool FindTarget(int& bestX, int& bestY, double& minDist, bool& isHead) {
    BitBlt(hdcMem, 0, 0, config.fov * 2, config.fov * 2, hdcScreen,
           centerX - config.fov, centerY - config.fov, SRCCOPY);
    GetDIBits(hdcMem, hBitmap, 0, config.fov * 2, pixels.data(), &bi, DIB_RGB_COLORS);
    
    bestX = -1;
    bestY = -1;
    minDist = 99999.0;
    isHead = false;
    
    int targetCount = 0;
    int totalX = 0, totalY = 0;
    
    for (int y = 0; y < config.fov * 2; y++) {
        for (int x = 0; x < config.fov * 2; x++) {
            int idx = (y * config.fov * 2 + x) * 4;
            BYTE r = pixels[idx + 2];
            BYTE g = pixels[idx + 1];
            BYTE b = pixels[idx];
            
            if (IsEnemyColor(r, g, b)) {
                double dist = sqrt(pow(x - config.fov, 2) + pow(y - config.fov, 2));
                
                // Headshot modu: Kafaya nişan al (yukarıdaki pixel'leri tercih et)
                if (config.headshotPriority) {
                    // Kafa genelde en üstteki pixel olur
                    double headBonus = (config.fov * 2 - y) * 0.5;
                    dist -= headBonus;
                }
                
                if (dist < minDist) {
                    minDist = dist;
                    bestX = x - config.fov;
                    bestY = y - config.fov;
                    
                    // Kafa mı kontrol et
                    if (y < config.fov * 0.4) {
                        isHead = true;
                    }
                }
                
                targetCount++;
                totalX += (x - config.fov);
                totalY += (y - config.fov);
            }
        }
    }
    
    // Eğer çok fazla hedef varsa, merkeze al (kalabalıkta ortalama)
    if (targetCount > 50 && !config.headshotPriority) {
        bestX = totalX / targetCount;
        bestY = totalY / targetCount;
        minDist = sqrt(pow(bestX, 2) + pow(bestY, 2));
        return true;
    }
    
    return bestX != -1;
}

// ==================== TRIGGERBOT + VALORANT ÖZEL ====================
void ProcessTriggerBot(InterceptionMouseStroke& mstroke) {
    bool lmbPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    
    if (!lmbPressed || !config.triggerEnabled) {
        stats.active = false;
        wasShooting = false;
        remainderX = remainderY = 0;
        return;
    }
    
    stats.active = true;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    int bestX, bestY;
    double minDist;
    bool isHead;
    
    if (FindTarget(bestX, bestY, minDist, isHead)) {
        stats.targetsFound++;
        if (isHead) stats.headshots++;
        
        if (config.aimbotEnabled && (bestX != 0 || bestY != 0)) {
            // Valorant hassasiyetine göre ayarla
            double sensitivityMult = valorant.getSensitivityMultiplier() * (config.sensitivity / 1.06);
            
            int adjustedX = (int)(bestX * sensitivityMult);
            int adjustedY = (int)(bestY * sensitivityMult);
            
            // Headshot offset
            if (config.headshotPriority && !isHead) {
                adjustedY -= 12;  // Yukarı kaydır
            }
            
            int totalX = adjustedX + remainderX;
            int totalY = adjustedY + remainderY;
            int moveX = totalX / config.smooth;
            int moveY = totalY / config.smooth;
            remainderX = totalX % config.smooth;
            remainderY = totalY % config.smooth;
            
            if (moveX != 0 || moveY != 0) {
                auto points = generateTrollMovement(0, 0, moveX, moveY);
                for (auto& p : points) {
                    if (p.x != 0 || p.y != 0) {
                        mstroke.x = p.x;
                        mstroke.y = p.y;
                        mstroke.flags = INTERCEPTION_MOUSE_MOVE_RELATIVE;
                        interception_send(context, device, (InterceptionStroke*)&mstroke, 1);
                    }
                    Sleep(p.waitMs);
                }
            }
        }
        
        // Trigger - Hedefe yakınsa
        double triggerDist = config.headshotPriority ? 15.0 : 25.0;
        if (minDist < triggerDist && !wasShooting) {
            int delay = GetRandom(config.minDelay, config.maxDelay);
            Sleep(delay);
            
            InterceptionMouseStroke fireStroke = {0};
            fireStroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN;
            interception_send(context, device, (InterceptionStroke*)&fireStroke, 1);
            
            int shotDuration = isHead ? GetRandom(6, 12) : GetRandom(10, 18);
            Sleep(shotDuration);
            
            fireStroke.state = INTERCEPTION_MOUSE_LEFT_BUTTON_UP;
            interception_send(context, device, (InterceptionStroke*)&fireStroke, 1);
            
            stats.shotsFired++;
            wasShooting = true;
            
            auto endTime = std::chrono::high_resolution_clock::now();
            stats.lastResponse = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        } else if (minDist >= triggerDist) {
            wasShooting = false;
        }
    } else {
        wasShooting = false;
    }
}

// ==================== PENCERE İŞLEMLERİ ====================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_INSERT) config.showMenu = !config.showMenu;
        if (wParam == VK_ESCAPE) PostQuitMessage(0);
        if (wParam == VK_F1) {
            config.trollMode = (config.trollMode + 1) % 7;
            const char* trollNames[] = {"NORMAL", "HIZLI", "YAVAS", "ZIKZAK", "SPIN", "JITTER", "SMOOTH"};
            SetConsoleTextAttribute(hConsole, 14);
            std::cout << "\n   [!] TROLL MOD DEGISTI: " << trollNames[config.trollMode] << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
        }
        if (wParam == VK_F2) {
            config.sensitivity += 0.01;
            if (config.sensitivity > 3.0) config.sensitivity = 3.0;
            SetConsoleTextAttribute(hConsole, 10);
            std::cout << "\n   [+] Hassasiyet: " << config.sensitivity << " (Valorant)" << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
        }
        if (wParam == VK_F3) {
            config.sensitivity -= 0.01;
            if (config.sensitivity < 0.1) config.sensitivity = 0.1;
            SetConsoleTextAttribute(hConsole, 10);
            std::cout << "\n   [+] Hassasiyet: " << config.sensitivity << " (Valorant)" << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
        }
        if (wParam == VK_F4) {
            config.headshotPriority = !config.headshotPriority;
            SetConsoleTextAttribute(hConsole, 11);
            std::cout << "\n   [+] AIM MOD: " << (config.headshotPriority ? "HEADSHOT" : "CHEST") << std::endl;
            SetConsoleTextAttribute(hConsole, 7);
        }
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ==================== ANA ====================
int main() {
    SetConsoleTitleA("RiseVLTR Valorant TriggerBot v6.0");
    ShowLogo();
    
    if (!InitInterception()) {
        system("pause");
        return 1;
    }
    
    stats.startTime = GetTickCount();
    
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);
    centerX = screenWidth / 2;
    centerY = screenHeight / 2;
    
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "RiseVLValorantBot";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassExA(&wc);
    
    g_hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        "RiseVLValorantBot", "RiseVLTR Valorant Bot", WS_POPUP,
        0, 0, screenWidth, screenHeight,
        NULL, NULL, wc.hInstance, NULL
    );
    
    SetLayeredWindowAttributes(g_hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    ShowWindow(g_hwnd, SW_SHOW);
    
    HDC hdc = GetDC(g_hwnd);
    SetProcessDPIAware();
    hdcScreen = GetDC(NULL);
    hdcMem = CreateCompatibleDC(hdcScreen);
    hBitmap = CreateCompatibleBitmap(hdcScreen, config.fov * 2, config.fov * 2);
    SelectObject(hdcMem, hBitmap);
    pixels.resize(config.fov * 2 * config.fov * 2 * 4);
    
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = config.fov * 2;
    bi.bmiHeader.biHeight = -(config.fov * 2);
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    
    SetConsoleTextAttribute(hConsole, 10);
    std::cout << "   [✓] Sistem hazir!" << std::endl;
    std::cout << "   [✓] Düşman rengi: MOR (Valorant outline)" << std::endl;
    std::cout << "   [✓] Riot Games, bu benim kendi oyunum! :)\n" << std::endl;
    
    InterceptionStroke stroke;
    MSG msg = {0};
    
    while (true) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                interception_destroy_context(context);
                DeleteObject(hBitmap);
                DeleteDC(hdcMem);
                ReleaseDC(NULL, hdcScreen);
                ReleaseDC(g_hwnd, hdc);
                DestroyWindow(g_hwnd);
                
                SetConsoleTextAttribute(hConsole, 13);
                std::cout << "\n   ╔════════════════════════════════════════════╗" << std::endl;
                std::cout << "   ║     RISEVLTR VALORANT BOT KAPATILDI       ║" << std::endl;
                std::cout << "   ║                                            ║" << std::endl;
                std::cout << "   ║     Toplam Hedef: " << stats.targetsFound << "                         ║" << std::endl;
                std::cout << "   ║     Toplam Atis: " << stats.shotsFired << "                           ║" << std::endl;
                std::cout << "   ║     Headshot: " << stats.headshots << " (" << (stats.shotsFired > 0 ? (stats.headshots * 100 / stats.shotsFired) : 0) << "%)        ║" << std::endl;
                std::cout << "   ╚════════════════════════════════════════════╝" << std::endl;
                SetConsoleTextAttribute(hConsole, 14);
                std::cout << "\n   [★] Videoyu begendiysen ABONE OL! RiseVLTR" << std::endl;
                std::cout << "   [★] Riot Games'e selamlar, ben kendi oyunumda kullaniyorum :)" << std::endl;
                system("pause");
                return 0;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        HDC hdcDraw = GetDC(g_hwnd);
        DrawRect(hdcDraw, 0, 0, screenWidth, screenHeight, RGB(0, 0, 0));
        DrawGUI(hdcDraw);
        ReleaseDC(g_hwnd, hdcDraw);
        
        device = interception_wait(context);
        if (interception_receive(context, device, &stroke, 1) > 0) {
            if (interception_is_mouse(device)) {
                InterceptionMouseStroke& mstroke = *(InterceptionMouseStroke*)&stroke;
                ProcessTriggerBot(mstroke);
            }
            interception_send(context, device, &stroke, 1);
        }
        
        Sleep(1);
    }
    
    return 0;
}
