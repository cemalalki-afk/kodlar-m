import os
import zipfile

# --- RiseVL-Sentry v9.8 | Masaüstü Hedefli Siber Analizör ---
USER_PROFILE = os.environ['USERPROFILE']
# Dosyayı direkt Masaüstü'ne kaydediyoruz
REPORT_FILE = os.path.join(USER_PROFILE, 'Desktop', 'dangers.txt')

DANGEROUS_EXTS = ['.jar', '.exe', '.bat', '.vbs', '.js', '.pyw']

MALWARE_SIGNS = {
    "powershell -ExecutionPolicy Bypass": "KRİTİK: Sistem Güvenliği Devre Dışı Bırakma",
    "WScript.Shell": "ORTA: Script Tabanlı Dosya Erişimi",
    "TEMP\\payload.exe": "KRİTİK: Geçici Klasörde Zararlı Yük",
    "reverse_shell": "ACİL: Uzaktan Erişim Girişimi",
    "Startup": "YÜKSEK: Başlangıca Sızma (Kalıcılık)",
    "socket.connect": "ORTA: Şüpheli Ağ Bağlantısı"
}

danger_report = []

def init_report():
    """Masaüstündeki rapor dosyasını sıfırlar"""
    try:
        with open(REPORT_FILE, "w", encoding="utf-8") as f:
            f.write("--- RiseVL-Sentry Operasyon Raporu ---\n")
        print(f"[*] Rapor dosyası hazır: {REPORT_FILE}")
    except Exception as e:
        print(f"[!] Dosya oluşturma hatası: {e}")

def scan_file(file_path):
    try:
        if file_path.endswith(('.bat', '.vbs', '.js', '.pyw')):
            with open(file_path, 'r', errors='ignore') as f:
                content = f.read()
                for sign, desc in MALWARE_SIGNS.items():
                    if sign in content:
                        report_entry = f"{desc} -> {file_path}"
                        danger_report.append(report_entry)
                        return desc
        
        elif file_path.endswith('.jar'):
            with zipfile.ZipFile(file_path, 'r') as jar:
                for file in jar.namelist():
                    if "payload" in file.lower() or "rat" in file.lower():
                        report_entry = f"TEHLİKELİ JAR: {file} -> {file_path}"
                        danger_report.append(report_entry)
                        return "Şüpheli JAR İçeriği"
    except: pass
    return None

def start_mission():
    init_report()
    print(f"--- RiseVL-Sentry v9.8 | Üs: Elazığ ---")
    print("1- Hızlı Tarama (Masaüstü/İndirilenler)\n2- Tam Tarama (Kullanıcı Klasörü)")
    
    secim = input("\nSeçimini yap (1/2): ")
    scan_paths = [os.path.join(USER_PROFILE, 'Downloads'), os.path.join(USER_PROFILE, 'Desktop')] if secim == "1" else [USER_PROFILE]

    print(f"\n[*] Tarama başlatıldı... Rapor şuraya çıkacak: {REPORT_FILE}")
    
    for path in scan_paths:
        for root, _, files in os.walk(path):
            for file in files:
                if os.path.splitext(file)[1].lower() in DANGEROUS_EXTS:
                    scan_file(os.path.join(root, file))

    print("\n" + "="*50)
    if not danger_report:
        msg = "[V] TEMİZ: Siber risk bulunamadı."
        print(msg)
        with open(REPORT_FILE, "a", encoding="utf-8") as f:
            f.write(msg + "\n")
    else:
        print(f"[!] {len(danger_report)} ADET RİSK BULUNDU!")
        with open(REPORT_FILE, "a", encoding="utf-8") as f:
            for report in danger_report:
                f.write(report + "\n")
        print(f"[*] Detaylar Masaüstü'ndeki 'dangers.txt' dosyasına yazıldı.")
    
    print("="*50)
    input("\nKapatmak için ENTER'a bas...")

if __name__ == "__main__":
    start_mission()