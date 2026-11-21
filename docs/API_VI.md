# Hướng dẫn API & Tự Động Hóa MotionFX (OBS Studio) 🚀

> **Tài liệu hướng dẫn điều khiển MotionFX từ xa qua OBS-WebSocket v5, Python (`obspython`), Lua (`obslua`), và Stream Deck / Touch Portal.**

---

## 📑 Mục lục
1. [Tổng quan cơ chế hoạt động](#1-tổng-quan-cơ-chế-hoạt-động)
2. [Bảng danh sách Preset & Thông số](#2-bảng-danh-sách-preset--thông-số)
3. [Phương pháp 1: OBS-WebSocket v5 (Điều khiển từ xa)](#3-phương-pháp-1-obs-websocket-v5-điều-khiển-từ-xa)
4. [Phương pháp 2: Viết Script Python trong OBS (`obspython`)](#4-phương-pháp-2-viết-script-python-trong-obs-obspython)
5. [Phương pháp 3: Viết Script Lua trong OBS (`obslua`)](#5-phương-pháp-3-viết-script-lua-trong-obs-obslua)
6. [Phương pháp 4: Tích hợp Stream Deck & Touch Portal](#6-phương-pháp-4-tích-hợp-stream-deck--touch-portal)

---

## 1. Tổng quan cơ chế hoạt động

MotionFX được xây dựng dưới dạng **Transition Source gốc của OBS** (`OBS_SOURCE_TYPE_TRANSITION` với ID `motionfx_transition`).

Trong OBS Studio:
Mỗi nguồn trong cảnh (Scene Item) đều có 2 chuyển cảnh riêng biệt:
- **Show Transition (Chuyển cảnh Hiện):** Kích hoạt khi nguồn được bật hiển thị (`visible = true`). MotionFX chạy chuyển động từ $0.0 \to 1.0$ đưa nguồn mượt mà vào khung hình.
- **Hide Transition (Chuyển cảnh Ẩn):** Kích hoạt khi nguồn bị tắt ẩn đi (`visible = false`). MotionFX tự động **đảo ngược dòng thời gian** ($1.0 \to 0.0$), đưa nguồn từ vị trí đang đứng lật/trượt thoát ra ngoài rồi mới biến mất hoàn toàn.

👉 **Điều này có nghĩa:** Bạn không cần phải viết hàm render phức tạp. Bất kỳ lệnh nào làm thay đổi trạng thái ẩn/hiện của nguồn (qua WebSocket, Python, Lua, Hotkey, Stream Deck) đều **tự động kích hoạt chuyển cảnh MotionFX**!

---

## 2. Bảng danh sách Preset & Thông số

### 🎭 Tên các mẫu chuyển động (Preset ID)
Điền đúng ID chuỗi (string) bên dưới khi gọi qua WebSocket hoặc Script:

| Preset ID | Tên hiển thị | Kiểu chuyển động |
| :--- | :--- | :--- |
| `3d_flip_x` | 3D Flip X | Lật thẻ 3D theo trục X thẳng đứng (từ 90° về 0°) |
| `3d_flip_y` | 3D Flip Y | Lật thẻ bài 3D theo trục Y ngang (từ 90° về 0°) |
| `3d_flip_diagonal` | 3D Flip Diagonal | Lật 3D theo đường chéo 2 trục kết hợp |
| `perspective_tilt` | Perspective Tilt | Nghiêng phối cảnh 3D có độ sâu camera |
| `zoom_spin` | Zoom Spin | Vừa xoay tròn 2D vừa phóng to (Twist & Zoom) |
| `blur_reveal` | Blur Focus | Làm mờ quang học Gaussian 9-tap rõ dần kết hợp mờ đục |
| `elastic_pop` | Elastic Pop | Nảy đàn hồi vật lý Overshoot cực mượt |
| `glitch_wipe` | Glitch Displace | Hiệu ứng quét nhiễu sóng Cyberpunk Glitch |
| `mask_reveal` | Mask Reveal (Chéo) | Quét mặt nạ góc chéo 45° có làm mềm biên (Feather) |
| `mask_radial` | Radial Iris Wipe | Mở ống kính tròn (Iris) từ tâm mở rộng ra |
| `mask_box` | Box Wipe | Mặt nạ hình chữ nhật mở rộng dần |
| `text_reveal` | Text Typewriter | Hiệu ứng máy đánh chữ (từng chữ cái một) |
| `character_stagger` | Character Stagger | Từng ký tự rơi vào theo bậc thang |
| `smooth_slide_left` | Slide Trái | Trượt mượt mà từ mép trái màn hình vào |
| `smooth_slide_right`| Slide Phải | Trượt mượt mà từ mép phải màn hình vào |
| `smooth_slide_up`   | Slide Lên | Trượt mượt mà từ mép dưới lên |
| `smooth_slide_down` | Slide Xuống | Trượt mượt mà từ mép trên xuống |
| `cinematic_fly_through`| 3D Fly-Through | Camera 3D bay xuyên không gian Z |

### 📐 Điểm neo xoay (Anchor / Pivot)
- `"center"`: Tâm giữa nguồn.
- `"top"`: Cạnh trên (lật từ trên gập xuống).
- `"bottom"`: Cạnh dưới (lật từ dưới đứng lên).
- `"left"`: Cạnh trái (mở như cánh cửa bên trái).
- `"right"`: Cạnh phải (mở từ cánh phải).

### ⏱️ Đường cong gia tốc (Easing)
`"ease_out"`, `"ease_in"`, `"ease_in_out"`, `"back_out"` (vọt lố), `"elastic_out"` (lò xo đàn hồi), `"bounce_out"` (nảy bóng tưng tưng), `"linear"`.

---

## 3. Phương pháp 1: OBS-WebSocket v5 (Điều khiển từ xa)

OBS Studio 28+ tích hợp sẵn OBS-WebSocket v5 (Cổng mặc định: `4455`).

### A. Gửi gói tin JSON kích hoạt Hiện / Ẩn Animation

#### Gói tin hiện nguồn (Chạy Show Animation):
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

#### Gói tin ẩn nguồn (Chạy Hide Animation đảo ngược):
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

### B. Sử dụng file mẫu Python có sẵn (`obs_websocket_client.py`)
Chúng tôi đã cung cấp sẵn script [`scripts/obs_websocket_client.py`](file:///c:/Users/Dell/Desktop/advanced%20transition/scripts/obs_websocket_client.py):
```bash
# Bật/tắt đảo trạng thái (Toggle)
python scripts/obs_websocket_client.py "Gaming Scene" "Webcam" toggle

# Kích hoạt Show
python scripts/obs_websocket_client.py "Gaming Scene" "AlertBox" show "mat_khau_obs"

# Kích hoạt Hide
python scripts/obs_websocket_client.py "Gaming Scene" "AlertBox" hide "mat_khau_obs"
```

### C. Sử dụng Node.js / JavaScript (`obs_websocket_client.js`)
Dành cho bot stream, web overlay hoặc Electron:
```javascript
const MotionFXClient = require('./scripts/obs_websocket_client.js');

async function main() {
    const client = new MotionFXClient('localhost', 4455, 'mat_khau_cua_ban');
    await client.connect();

    // Hiện Webcam với hiệu ứng MotionFX
    await client.showItem('Gaming Scene', 'Webcam');

    // Chờ 5 giây rồi ẩn đi với hiệu ứng lật ngược
    setTimeout(async () => {
        await client.hideItem('Gaming Scene', 'Webcam');
        client.close();
    }, 5000);
}
main();
```

---

## 4. Phương pháp 2: Viết Script Python trong OBS (`obspython`)

### A. Giao diện điều khiển có sẵn (Plug & Play)
1. Mở OBS Studio -> Chọn menu **Tools -> Scripts**.
2. Sang tab **Python Scripts**, bấm dấu **(+)** và chọn file [`scripts/motionfx_controller.py`](file:///c:/Users/Dell/Desktop/advanced%20transition/scripts/motionfx_controller.py).
3. Chọn Cảnh, chọn Nguồn, chọn Preset (vd: `3d_flip_y`), chỉnh thời lượng (`500 ms`).
4. Bấm **✨ Apply MotionFX to Scene Item** để áp dụng tự động!

### B. Code Python tự viết trong OBS:
```python
import obspython as obs

# Hàm gán MotionFX vào 1 Scene Item
def gan_motionfx_cho_nguon(ten_canh, ten_nguon, preset="3d_flip_x", thoi_luong_ms=500):
    scene_src = obs.obs_get_source_by_name(ten_canh)
    scene = obs.obs_scene_from_source(scene_src)
    item = obs.obs_scene_find_source(scene, ten_nguon)

    # Cấu hình cài đặt
    settings = obs.obs_data_create()
    obs.obs_data_set_string(settings, "preset", preset)
    obs.obs_data_set_int(settings, "duration", thoi_luong_ms)

    # Tạo transition MotionFX
    tr = obs.obs_source_create_private("motionfx_transition", "MotionFX_Instance", settings)

    # Gán vào cả Show và Hide
    obs.obs_sceneitem_set_show_transition(item, tr)
    obs.obs_sceneitem_set_show_transition_duration(item, thoi_luong_ms)
    obs.obs_sceneitem_set_hide_transition(item, tr)
    obs.obs_sceneitem_set_hide_transition_duration(item, thoi_luong_ms)

    # Giải phóng bộ nhớ
    obs.obs_source_release(tr)
    obs.obs_data_release(settings)
    obs.obs_source_release(scene_src)

# Hàm kích hoạt chạy chuyển cảnh
def kich_hoat_hien(ten_canh, ten_nguon, hien=True):
    scene_src = obs.obs_get_source_by_name(ten_canh)
    scene = obs.obs_scene_from_source(scene_src)
    item = obs.obs_scene_find_source(scene, ten_nguon)
    if item:
        obs.obs_sceneitem_set_visible(item, hien)
    obs.obs_source_release(scene_src)
```

---

## 5. Phương pháp 3: Viết Script Lua trong OBS (`obslua`)

Lua chạy mượt mà, **không cần cài Python**, tích hợp sẵn 100% trong OBS Studio.

### A. Giao diện Lua Controller có sẵn
1. Trong OBS Studio, vào **Tools -> Scripts**.
2. Bấm dấu **(+)** và thêm file [`scripts/motionfx_controller.lua`](file:///c:/Users/Dell/Desktop/advanced%20transition/scripts/motionfx_controller.lua).
3. Bảng điều khiển trực quan xuất hiện ngay lập tức!

### B. Code Lua tự viết:
```lua
local obs = obslua

-- Hàm gán MotionFX
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

-- Hàm đảo trạng thái Ẩn/Hiện
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

## 6. Phương pháp 4: Tích hợp Stream Deck & Touch Portal

### Cách 1: Dùng Phím tắt (Hotkeys)
1. Thêm `scripts/motionfx_controller.lua` vào **Tools -> Scripts**.
2. Mở **Cài đặt OBS -> Phím tắt (Settings -> Hotkeys)**.
3. Tìm `MotionFX`:
   - `MotionFX: Toggle Item Visibility`
   - `MotionFX: Show Item (Animate In)`
   - `MotionFX: Hide Item (Animate Out)`
4. Đặt phím tắt tùy ý (ví dụ: `Ctrl+Shift+F1`).
5. Trên phần mềm **Stream Deck** hoặc **Touch Portal**, gán phím tắt đó vào nút bấm!

### Cách 2: Dùng hành động gốc "Source Visibility" của Stream Deck
1. Trong ứng dụng Stream Deck, kéo action **OBS Studio -> Source Visibility** vào 1 phím.
2. Chọn Cảnh và Nguồn mong muốn.
3. Mỗi khi bấm phím, OBS sẽ đổi trạng thái nguồn và tự động kích hoạt chuyển cảnh MotionFX 3D lật vào hoặc lật ra!
