import openai
import json
import os
from datetime import datetime

# --- DEEPSEEK YAPILANDIRMA ---
# API Key ve URL düzeltildi
DEEPSEEK_API_KEY = "sk-fc9e64bc485c4d35ab21030d77a046ab"
DEEPSEEK_BASE_URL = "https://api.deepseek.com"

client = openai.OpenAI(
    api_key=DEEPSEEK_API_KEY,
    base_url=DEEPSEEK_BASE_URL
)

MEMORY_FILE = "astron_hafiza_deepseek.json"

# --- HAFIZA SİSTEMİ ---
def hafiza_yukle():
    if os.path.exists(MEMORY_FILE):
        try:
            with open(MEMORY_FILE, "r", encoding="utf-8") as f:
                return json.load(f)
        except Exception:
            return []
    return []

def hafiza_kaydet(history):
    try:
        with open(MEMORY_FILE, "w", encoding="utf-8") as f:
            json.dump(history, f, ensure_ascii=False, indent=4)
    except Exception as e:
        print(f"\n[SİSTEM]: Kayıt hatası: {e}")

# --- TOKEN SAYACI ---
def token_say(text):
    return len(str(text)) // 4

def istatistik_goster(toplam_girdi, toplam_cikti):
    print("\n" + "="*50)
    print(f"📊 ASTRON İSTATİSTİKLERİ")
    print("="*50)
    print(f"📥 Toplam Girdi Token: {toplam_girdi:,}")
    print(f"📤 Toplam Çıktı Token: {toplam_cikti:,}")
    print(f"💰 Toplam Kullanım: {toplam_girdi + toplam_cikti:,}")
    print("="*50 + "\n")

# --- SİSTEM PROMPTU ---
SYSTEM_PROMPT = """Sen Risevl'in asistanı ASTRON'sun. 
Risevl 11 yaşında bir siber güvenlik dâhisi. 
Konuşma tarzın: Teknik konularda uzman, normalde samimi ve kanka gibi. 
Elazığlı olduğunu ve acı (Sriracha) direncini biliyorsun."""

# --- ANA ASTRON FONKSİYONU ---
def astron_konus(user_input, chat_history):
    messages = [{"role": "system", "content": SYSTEM_PROMPT}]
    
    # Son 10 konuşmayı bağlam olarak ekle
    for h in chat_history[-10:]:
        messages.append({"role": "user", "content": h["user"]})
        messages.append({"role": "assistant", "content": h["astron"]})
    
    messages.append({"role": "user", "content": user_input})
    
    try:
        response = client.chat.completions.create(
            model="deepseek-chat",
            messages=messages,
            temperature=0.7,
            max_tokens=2048
        )
        
        astron_reply = response.choices[0].message.content
        input_tokens = token_say(user_input) + token_say(SYSTEM_PROMPT)
        output_tokens = token_say(astron_reply)
        
        return astron_reply, input_tokens, output_tokens
        
    except Exception as e:
        return f"Siber bir arıza var kanka: {e}", 0, 0

# --- ANA PROGRAM ---
toplam_girdi_token = 0
toplam_cikti_token = 0

print("\n" + "="*50)
print("   🤖 ASTRON V2.0 - DEEPSEEK EDITION 🤖")
print("="*50)

chat_history = hafiza_yukle()
print("[ASTRON]: Selam Risevl! Elazığ Ataşehir'den siber dünyaya akmaya hazır mısın? 🚀")

# Ana Döngü
try:
    while True:
        user_input = input("\nRisevl > ")
        
        if user_input.lower() in ["exit", "kapat", "görüşürüz", "bay"]:
            hafiza_kaydet(chat_history)
            istatistik_goster(toplam_girdi_token, toplam_cikti_token)
            print("[ASTRON]: Görüşürüz kral, sistemin 229 FPS'te kalsın! 🌶️")
            break
        
        if not user_input.strip():
            continue
            
        astron_reply, input_tokens, output_tokens = astron_konus(user_input, chat_history)
        
        toplam_girdi_token += input_tokens
        toplam_cikti_token += output_tokens
        
        chat_history.append({
            "user": user_input, 
            "astron": astron_reply, 
            "timestamp": str(datetime.now())
        })
        
        if len(chat_history) % 5 == 0:
            hafiza_kaydet(chat_history)
        
        print(f"\n[ASTRON]: {astron_reply}")
        print(f"📊 [TOKEN]: {input_tokens + output_tokens}")

except KeyboardInterrupt:
    hafiza_kaydet(chat_history)
    print("\n[ASTRON]: Güle güle!")

except Exception as e:
    print(f"\n[KRİTİK HATA]: {e}")

# TERMİNALİN KAPANMASINI ENGELLEYEN SATIR:
print("\n" + "="*50)
input("Program bitti. Kapatmak için ENTER'a bas...")