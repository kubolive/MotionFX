# Huong dan API & Tu Dong Hoa MotionFX (OBS Studio)

> **Tai lieu huong dan dieu khien MotionFX tu xa qua OBS-WebSocket v5, Python (`obspython`), Lua (`obslua`), va Stream Deck / Touch Portal.**

---

## Muc luc
1. [Tong quan co che hoat dong](#1-tong-quan-co-che-hoat-dong)
2. [Bang danh sach Preset & Thong so](#2-bang-danh-sach-preset--thong-so)
3. [Tuy bien thong so thu cong (Custom Parameter Overrides)](#3-tuy-bien-thong-so-thu-cong-custom-parameter-overrides)
4. [Phuong phap 1: OBS-WebSocket v5 (Dieu khien tu xa)](#4-phuong-phap-1-obs-websocket-v5-dieu-khien-tu-xa)
5. [Phuong phap 2: Viet Script Python trong OBS (`obspython`)](#5-phuong-phap-2-viet-script-python-trong-obs-obspython)
6. [Phuong phap 3: Viet Script Lua trong OBS (`obslua`)](#6-phuong-phap-3-viet-script-lua-trong-obs-obslua)
7. [Phuong phap 4: Tich hop Stream Deck & Touch Portal](#7-phuong-phap-4-tich-hop-stream-deck--touch-portal)

---

## 1. Tong quan co che hoat dong

MotionFX duoc xay dung duoi dang **Transition Source goc cua OBS** (`OBS_SOURCE_TYPE_TRANSITION` voi ID `motionfx_transition`).

Trong OBS Studio:
Moi nguon trong canh (Scene Item) deu co 2 chuyen canh rieng biet:
- **Show Transition (Chuyen canh Hien):** Kich hoat khi nguon duoc bat hien thi (`visible = true`). MotionFX chay chuyen dong tu 0.0 -> 1.0 dua nguon muot ma vao khung hinh.
- **Hide Transition (Chuyen canh An):** Kich hoat khi nguon bi tat an di (`visible = false`). MotionFX tu dong **dao nguoc dong thoi gian** (1.0 -> 0.0), dua nguon tu vi tri dang dung lat/thoat ra ngoai roi moi bien mat hoan toan.

Bat ky lenh nao lam thay doi trang thai an/hien cua nguon (qua WebSocket, Python, Lua, Hotkey, Stream Deck) deu **tu dong kich hoat chuyen canh MotionFX**.

---

## 2. Bang danh sach Preset & Thong so

### Ten cac mau chuyen dong (Preset ID)
Dien dung ID chuoi (string) ben duoi khi goi qua WebSocket hoac Script:

| Preset ID | Ten hien thi | Kieu chuyen dong |
| :--- | :--- | :--- |
| `3d_flip_x` | 3D Flip X | Lat the 3D theo truc X thang dung (tu 90° ve 0°) |
| `3d_flip_y` | 3D Flip Y | Lat the bai 3D theo truc Y ngang (tu 90° ve 0°) |
| `3d_flip_diagonal` | 3D Flip Diagonal | Lat 3D theo duong cheo 2 truc ket hop |
| `perspective_tilt` | Perspective Tilt | Nghieng phoi canh 3D co do sau camera |
| `zoom_spin` | Zoom Spin | Vua xoay tron 2D vua phong to (Twist & Zoom) |
| `blur_reveal` | Blur Focus | Lam mo quang hoc Gaussian 9-tap ro dan ket hop mo duc |
| `elastic_pop` | Elastic Pop | Nay dan hoi vat ly Overshoot cuc muot |
| `glitch_wipe` | Glitch Displace | Hieu ung quet nhieu song Cyberpunk Glitch |
| `mask_reveal` | Mask Reveal (Cheo) | Quet mat na goc cheo 45° co lam mem bien (Feather) |
| `mask_radial` | Radial Iris Wipe | Mo ong kinh tron (Iris) tu tam mo rong ra |
| `mask_box` | Box Wipe | Mat na hinh chu nhat mo rong dan |
| `text_reveal` | Text Typewriter | Hieu ung may danh chu (tung chu cai mot) |
| `character_stagger` | Character Stagger | Tung ky tu roi vao theo bac thang |
| `smooth_slide_left` | Slide Trai | Truot muot ma tu mep trai man hinh vao |
| `smooth_slide_right`| Slide Phai | Truot muot ma tu mep phai man hinh vao |
| `smooth_slide_up`   | Slide Len | Truot muot ma tu mep duoi len |
| `smooth_slide_down` | Slide Xuong | Truot muot ma tu mep tren xuong |
| `cinematic_fly_through`| 3D Fly-Through | Camera 3D bay xuyen khong gian Z |

### Diem neo xoay (Anchor / Pivot)
- `"center"`: Tam giua nguon.
- `"top"`: Canh tren (lat tu tren gap xuong).
- `"bottom"`: Canh duoi (lat tu duoi dung len).
- `"left"`: Canh trai (mo nhu canh cua ben trai).
- `"right"`: Canh phai (mo tu canh phai).

### Duong cong gia toc (Easing)
`"ease_out"`, `"ease_in"`, `"ease_in_out"`, `"back_out"` (vot lo), `"elastic_out"` (lo xo dan hoi), `"bounce_out"` (nay bong tung tung), `"linear"`.

---

## 3. Tuy bien thong so thu cong (Custom Parameter Overrides)

Ngoai cac preset co san, MotionFX cho phep ban tu do tuy bien va ghep noi bat ky thanh phan animation nao qua co `"custom_override": true`.

### Bang danh sach cac khoa can thiep thu cong

| Khoa cai dat (Key) | Kieu | Mac dinh | Y nghia & Mo ta |
| :--- | :--- | :--- | :--- |
| `custom_override` | `bool` | `false` | Bat/tat che do can thiep thong so thu cong. |
| `duration` | `int` | `500` | Thoi luong animation (mili-giay). |
| `anchor_point` | `string` | `"center"` | Diem neo: `"center"`, `"top"`, `"bottom"`, `"left"`, `"right"`. |
| `easing` | `string` | `"ease_out"` | Duong cong toc do (`elastic_out`, `back_out`...). |
| `enable_3d` | `bool` | `false` | Bat che do lat khong gian 3D Euler. |
| `rot_x` | `double` | `90.0` | Goc lat ban dau truc doc X (vi du: 45.0, 90.0, 180.0). |
| `rot_y` | `double` | `0.0` | Goc lat ban dau truc ngang Y (lat the bai). |
| `enable_rotation`| `bool` | `false` | Bat xoay mat phang 2D Z. |
| `rot_z` | `double` | `0.0` | Goc xoay 2D ban dau (do). |
| `enable_slide` | `bool` | `false` | Bat truot vi tri. |
| `slide_x` | `double` | `0.0` | Do lech bat dau theo truc ngang X (pixel). |
| `slide_y` | `double` | `0.0` | Do lech bat dau theo truc doc Y (pixel). |
| `slide_z` | `double` | `0.0` | Do sau trong khong gian 3D Z (pixel). |
| `enable_scale` | `bool` | `false` | Bat phong to / thu nho (Zoom). |
| `scale_start` | `double` | `0.0` | Ti le kich thuoc ban dau (0.5 = 50%). |
| `scale_end` | `double` | `1.0` | Ti le kich thuoc ket thuc. |
| `enable_fade` | `bool` | `false` | Bat mo duc / trong suot (Opacity). |
| `fade_start` | `double` | `0.0` | Do trong suot ban dau (0.0 = vo hinh). |
| `fade_end` | `double` | `1.0` | Do trong suot ket thuc (1.0 = hien ro). |
| `enable_blur` | `bool` | `false` | Bat lam mo quang hoc Gaussian. |
| `blur_radius` | `double` | `12.0` | Ban kinh mo (1.0 den 30.0 px). |
| `enable_mask` | `bool` | `false` | Bat mat na quet reveal (Shader Mask). |
| `mask_type` | `int` | `1` | `1`: Cheo, `2`: Tron Iris, `3`: Hop Box, `4`: Typewriter, `5`: Stagger. |
| `mask_angle` | `double` | `45.0` | Goc quet mat na cheo (do). |
| `mask_feather` | `double` | `0.05` | Do mem vien cat (0.005 den 0.25). |

### Vi du code Python can thiep custom:
```python
settings = obs.obs_data_create()
obs.obs_data_set_bool(settings, "custom_override", True)
obs.obs_data_set_bool(settings, "enable_3d", True)
obs.obs_data_set_double(settings, "rot_x", 45.0)       # Lat nghieng 45 do
obs.obs_data_set_double(settings, "rot_y", 30.0)       # Nghieng truc Y 30 do
obs.obs_data_set_string(settings, "anchor_point", "bottom")
obs.obs_data_set_bool(settings, "enable_blur", True)
obs.obs_data_set_double(settings, "blur_radius", 18.0)
obs.obs_data_set_string(settings, "easing", "elastic_out")
obs.obs_data_set_int(settings, "duration", 750)

obs.obs_source_update(transition_source, settings)
obs.obs_data_release(settings)
```

---

## 4. Phuong phap 1: OBS-WebSocket v5 (Dieu khien tu xa)

OBS Studio 28+ tich hop san OBS-WebSocket v5 (Cong mac dinh: `4455`).

### A. Goi tin JSON kich hoat Hien / An Animation

#### Goi tin hien nguon (Chay Show Animation):
```json
{
  "op": 6,
  "d": {
    "requestType": "SetSceneItemEnabled",
    "requestId": "motionfx-req-1",
    "requestData": {
      "sceneName": "Main Scene",
      "sceneItemId": 42,
      "sceneItemEnabled": true
    }
  }
}
```

#### Goi tin an nguon (Chay Hide Animation dao nguoc):
```json
{
  "op": 6,
  "d": {
    "requestType": "SetSceneItemEnabled",
    "requestId": "motionfx-req-2",
    "requestData": {
      "sceneName": "Main Scene",
      "sceneItemId": 42,
      "sceneItemEnabled": false
    }
  }
}
```

### B. Su dung file mau Python co san (`obs_websocket_client.py`)
File script co san tai `scripts/obs_websocket_client.py`:
```bash
# Bat/tat dao trang thai (Toggle)
python scripts/obs_websocket_client.py "Gaming Scene" "Webcam" toggle

# Kich hoat Show
python scripts/obs_websocket_client.py "Gaming Scene" "AlertBox" show "mat_khau_obs"

# Kich hoat Hide
python scripts/obs_websocket_client.py "Gaming Scene" "AlertBox" hide "mat_khau_obs"
```

### C. Su dung Node.js / JavaScript (`obs_websocket_client.js`)
File script co san tai `scripts/obs_websocket_client.js`:
```javascript
const MotionFXClient = require('./scripts/obs_websocket_client.js');

async function main() {
    const client = new MotionFXClient('localhost', 4455, 'mat_khau_cua_ban');
    await client.connect();

    // Hien Webcam voi hieu ung MotionFX
    await client.showItem('Gaming Scene', 'Webcam');

    // Cho 5 giay roi an di voi hieu ung lat nguoc
    setTimeout(async () => {
        await client.hideItem('Gaming Scene', 'Webcam');
        client.close();
    }, 5000);
}
main();
```

---

## 5. Phuong phap 2: Viet Script Python trong OBS (`obspython`)

### A. Giao dien dieu khien co san
1. Mo OBS Studio -> Chon menu **Tools -> Scripts**.
2. Sang tab **Python Scripts**, bam dau **(+)** va chon file `scripts/motionfx_controller.py`.
3. Chon Canh, chon Nguon, chon Preset (vd: `3d_flip_y`), chinh thoi luong (`500 ms`).
4. Bam **Apply MotionFX to Scene Item** de ap dung tu dong.

### B. Code Python tu viet trong OBS:
```python
import obspython as obs

# Ham gan MotionFX vao 1 Scene Item
def gan_motionfx_cho_nguon(ten_canh, ten_nguon, preset="3d_flip_x", thoi_luong_ms=500):
    scene_src = obs.obs_get_source_by_name(ten_canh)
    scene = obs.obs_scene_from_source(scene_src)
    item = obs.obs_scene_find_source(scene, ten_nguon)

    # Cau hinh cai dat
    settings = obs.obs_data_create()
    obs.obs_data_set_string(settings, "preset", preset)
    obs.obs_data_set_int(settings, "duration", thoi_luong_ms)

    # Tao transition MotionFX
    tr = obs.obs_source_create_private("motionfx_transition", "MotionFX_Instance", settings)

    # Gan vao ca Show va Hide
    obs.obs_sceneitem_set_show_transition(item, tr)
    obs.obs_sceneitem_set_show_transition_duration(item, thoi_luong_ms)
    obs.obs_sceneitem_set_hide_transition(item, tr)
    obs.obs_sceneitem_set_hide_transition_duration(item, thoi_luong_ms)

    # Giai phong bo nho
    obs.obs_source_release(tr)
    obs.obs_data_release(settings)
    obs.obs_source_release(scene_src)

# Ham kich hoat chay chuyen canh
def kich_hoat_hien(ten_canh, ten_nguon, hien=True):
    scene_src = obs.obs_get_source_by_name(ten_canh)
    scene = obs.obs_scene_from_source(scene_src)
    item = obs.obs_scene_find_source(scene, ten_nguon)
    if item:
        obs.obs_sceneitem_set_visible(item, hien)
    obs.obs_source_release(scene_src)
```

---

## 6. Phuong phap 3: Viet Script Lua trong OBS (`obslua`)

Lua tich hop san 100% trong OBS Studio, khong can cai them Python.

### A. Giao dien Lua Controller co san
1. Trong OBS Studio, vao **Tools -> Scripts**.
2. Bam dau **(+)** va them file `scripts/motionfx_controller.lua`.
3. Bang dieu khien truc quan xuat hien ngay lap tuc.

### B. Code Lua tu viet:
```lua
local obs = obslua

-- Ham gan MotionFX
local function gan_motionfx(ten_canh, ten_nguon, preset, thoi_luong)
    local scene_src = obs.obs_get_source_by_name(ten_canh)
    local scene = obs.obs_scene_from_source(scene_src)
    local item = obs.obs_scene_find_source(scene, ten_nguon)

    local settings = obs.obs_data_create()
    obs.obs_data_set_string(settings, "preset", preset or "3d_flip_x")
    obs.obs_data_set_int(settings, "duration", thoi_luong or 500)

    local tr = obs.obs_source_create_private("motionfx_transition", "MotionFX", settings)
    obs.obs_sceneitem_set_show_transition(item, tr)
    obs.obs_sceneitem_set_show_transition_duration(item, thoi_luong)
    obs.obs_sceneitem_set_hide_transition(item, tr)
    obs.obs_sceneitem_set_hide_transition_duration(item, thoi_luong)

    obs.obs_source_release(tr)
    obs.obs_data_release(settings)
    obs.obs_source_release(scene_src)
end

-- Ham dao trang thai An/Hien
local function bat_tat_nguon(ten_canh, ten_nguon)
    local scene_src = obs.obs_get_source_by_name(ten_canh)
    local scene = obs.obs_scene_from_source(scene_src)
    local item = obs.obs_scene_find_source(scene, ten_nguon)
    if item then
        local dang_hien = obs.obs_sceneitem_visible(item)
        obs.obs_sceneitem_set_visible(item, not dang_hien)
    end
    obs.obs_source_release(scene_src)
end
```

---

## 7. Phuong phap 4: Tich hop Stream Deck & Touch Portal

### Cach 1: Dung Phim tat (Hotkeys)
1. Them `scripts/motionfx_controller.lua` vao **Tools -> Scripts**.
2. Mo **Cai dat OBS -> Phim tat (Settings -> Hotkeys)**.
3. Tim `MotionFX`:
   - `MotionFX: Toggle Item Visibility`
   - `MotionFX: Show Item (Animate In)`
   - `MotionFX: Hide Item (Animate Out)`
4. Dat phim tat tuy y (vi du: `Ctrl+Shift+F1`).
5. Tren phan mem **Stream Deck** hoac **Touch Portal**, gan phim tat do vao nut bam.

### Cach 2: Dung hanh dong goc "Source Visibility" cua Stream Deck
1. Trong ung dung Stream Deck, keo action **OBS Studio -> Source Visibility** vao 1 phim.
2. Chon Canh va Nguon mong muon.
3. Moi khi bam phim, OBS se doi trang thai nguon va tu dong kich hoat chuyen canh MotionFX 3D lat vao hoac lat ra.
