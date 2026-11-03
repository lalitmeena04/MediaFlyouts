"""Builds the Partner Center listing import for an add-on in all 11 languages.

An add-on listing has three fields: Title (required), Description (200 chars)
and StoreLogo300x300. The first export of a new add-on has only the "default"
column; this script adds one column per language (the import creates those
listings), fills Title and Description, and points the icon row at the PNG in
tools/store/addon-import/. Upload that folder with
"Import listings > Upload folder" on the add-on's submission page.

    python tools\\store\\make_addon_listing_csv.py pro listingData-<addon>-<submission>.csv

The first argument picks the add-on text below. Image cells must hold the path
including the root folder name ("addon-import/pro-icon.png").
"""
import csv
import sys
from pathlib import Path

HERE = Path(__file__).parent
OUT_DIR = HERE / "addon-import"
LANGS = ["en-us", "ar", "zh-hans", "fr-fr", "de-de", "hi", "id", "ja", "ko", "pt-br", "es-es"]
LIMITS = {"Title": 100, "Description": 200}

# ---------------------------------------------------------------- content ---

ADDONS = {}

ADDONS["pro"] = dict(
    icon="pro-icon.png",
    text={
        "en-us": dict(
            Title="MediaFlyouts Pro",
            Description="Unlocks every Pro feature for good: live audio visualizer, per-app volume mixer, peripheral battery flyouts, Now Playing styles and taskbar widget options. One-time purchase, no subscription.",
        ),
        "de-de": dict(
            Title="MediaFlyouts Pro",
            Description="Schaltet alle Pro-Funktionen dauerhaft frei: Live-Visualizer, Lautstärkemixer pro App, Akku-Flyouts für Peripheriegeräte, Now-Playing-Stile und Taskleisten-Widget-Optionen. Einmalkauf, kein Abo.",
        ),
        "fr-fr": dict(
            Title="MediaFlyouts Pro",
            Description="Débloque toutes les fonctions Pro pour toujours : visualiseur audio, mixeur de volume par appli, flyouts de batterie des périphériques, styles Lecture en cours et options du widget. Achat unique.",
        ),
        "es-es": dict(
            Title="MediaFlyouts Pro",
            Description="Desbloquea todas las funciones Pro para siempre: visualizador de audio, mezclador por aplicación, flyouts de batería de periféricos, estilos de Reproduciendo y opciones del widget. Pago único.",
        ),
        "pt-br": dict(
            Title="MediaFlyouts Pro",
            Description="Desbloqueia todos os recursos Pro para sempre: visualizador de áudio, mixer por app, flyouts de bateria de periféricos, estilos de Tocando agora e opções do widget. Compra única, sem assinatura.",
        ),
        "id": dict(
            Title="MediaFlyouts Pro",
            Description="Membuka semua fitur Pro selamanya: visualizer audio langsung, mixer volume per aplikasi, flyout baterai periferal, gaya Sedang Diputar, dan opsi widget taskbar. Sekali beli, tanpa langganan.",
        ),
        "ja": dict(
            Title="MediaFlyouts Pro",
            Description="プロ版の機能をすべて永久に解除：ライブ ビジュアライザー、アプリ別音量ミキサー、周辺機器のバッテリー フライアウト、再生中スタイル、タスクバー ウィジェットのオプション。買い切り、サブスクリプションなし。",
        ),
        "ko": dict(
            Title="MediaFlyouts Pro",
            Description="모든 프로 기능을 영구 해제: 라이브 시각화, 앱별 볼륨 믹서, 주변기기 배터리 플라이아웃, 지금 재생 중 스타일, 작업 표시줄 위젯 옵션. 1회 구매, 구독 없음.",
        ),
        "zh-hans": dict(
            Title="MediaFlyouts Pro",
            Description="永久解锁全部专业版功能：实时音频可视化、按应用音量混音器、外设电量弹出面板、正在播放样式和任务栏小组件选项。一次性购买，无需订阅。",
        ),
        "hi": dict(
            Title="MediaFlyouts Pro",
            Description="सभी प्रो सुविधाएँ हमेशा के लिए अनलॉक करें: लाइव विज़ुअलाइज़र, प्रति-ऐप वॉल्यूम मिक्सर, पेरिफ़ेरल बैटरी फ्लाईआउट, नाउ प्लेइंग स्टाइल और टास्कबार विजेट विकल्प। एक बार ख़रीदें, कोई सदस्यता नहीं।",
        ),
        "ar": dict(
            Title="MediaFlyouts Pro",
            Description="يفتح جميع ميزات برو للأبد: مصوّر صوتي مباشر، خلاط صوت لكل تطبيق، نوافذ بطارية الأجهزة الطرفية، أنماط «قيد التشغيل» وخيارات أداة شريط المهام. شراء لمرة واحدة، بدون اشتراك.",
        ),
    },
)

# --------------------------------------------------------------- generate ---

if len(sys.argv) != 3 or sys.argv[1] not in ADDONS:
    sys.exit(f"usage: {Path(sys.argv[0]).name} <{'|'.join(ADDONS)}> <export.csv>")
addon = ADDONS[sys.argv[1]]
src = Path(sys.argv[2])
out = OUT_DIR / f"{sys.argv[1]}-listing-import.csv"

missing = [l for l in LANGS if l not in addon["text"]]
if missing:
    sys.exit(f"no text for: {missing}")
problems = [f"{lang} {k}: {len(v)} > {LIMITS[k]}" for lang, t in addon["text"].items()
            for k, v in t.items() if len(v) > LIMITS[k]]
if problems:
    sys.exit("limits:\n  " + "\n  ".join(problems))

with src.open(encoding="utf-8-sig", newline="") as f:
    rows = [r for r in csv.reader(f) if any(c.strip() for c in r)]

header = rows[0]
for lang in LANGS:
    if lang not in header:
        header.append(lang)
col = {name: i for i, name in enumerate(header)}

icon = OUT_DIR / addon["icon"]
if not icon.exists():
    sys.exit(f"icon missing: {icon}")

changed = 0
for row in rows[1:]:
    row.extend([""] * (len(header) - len(row)))
    field = row[col["Field"]]
    for lang in LANGS:
        if field in LIMITS:
            v = addon["text"][lang][field]
        elif field == "StoreLogo300x300":
            v = f"{OUT_DIR.name}/{icon.name}"
        else:
            continue
        if row[col[lang]] != v:
            row[col[lang]] = v
            changed += 1

OUT_DIR.mkdir(exist_ok=True)
with out.open("w", encoding="utf-8-sig", newline="") as f:
    w = csv.writer(f, lineterminator="\r\n", quoting=csv.QUOTE_MINIMAL)
    w.writerows(rows)
print(f"wrote {OUT_DIR.name}/{out.name} ({len(rows) - 1} rows, {len(LANGS)} languages, {changed} cells) + {icon.name}")
