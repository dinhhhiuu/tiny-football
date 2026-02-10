## 🧑‍💻 PHÂN CÔNG NHÓM 3 NGƯỜI – TINY FOOTBALL (SDL2)
###  🔴 NGUYÊN TẮC CHIA

- A: lo nền móng + vòng đời game
- B: lo điều khiển + cầu thủ
- C: lo bóng + vật lý + điểm

## 🅰️ NGƯỜI A – CORE + FRAMEWORK (xương sống game)

### Nhiệm vụ chính
- (1) SDL2 Core
    - Init SDL2
    - Window + Renderer
    - Game loop chuẩn:
    - handleInput()
    - update()
    - render()

- (2) Time & FPS
    - Fix 60 FPS
    - Delta time (nếu có)
- (3) Quản lý scene/game state
    - Menu (optional)
    - Play
    - Exit
- (4) File structure
```
src/
 ├── main.cpp
 ├── game.cpp / game.h
 ├── config.h
```
### Deliverable
- Chạy được màn hình đen + loop
- Có thể gọi hàm update/render từ file khác

## 🅱️ NGƯỜI B – PLAYER + INPUT (ăn nhiều điểm trực tiếp)

👉 B là người ăn điểm cầu thủ + keyboard

### Nhiệm vụ chính

- (1) Player object
    - Struct Player {x, y, w, h, speed}
    - Render player
- (2) Keyboard input
    - Player 1: W A S D
    - Player 2: ← ↑ ↓ →
- (3) Nhiều cầu thủ
    - Tạo mảng players[2]
    - Update độc lập
- (4) Activate player / group
    - Mỗi người điều khiển 1 player
    - Hoặc TAB đổi player
- (5) Giữ player trong màn hình
    - Không cho chạy ra ngoài
### Deliverable
- 2 player di chuyển mượt
- Điều khiển độc lập
- 📌 Ăn trọn:
    - Cầu thủ (2đ)
    - Keyboard (3đ)
    - 2 người chơi (1đ)

## 🅲 NGƯỜI C – BALL + COLLISION + SCORE (logic game)

### Nhiệm vụ chính

    - (1) Ball object
        - Vị trí, vận tốc
        - Update tự động
    - (2) Va chạm biên
        - 4 cạnh màn hình
        - Phản xạ đúng
    - (3) Va chạm ball ↔ player
        - Bóng bật khi đụng cầu thủ
    - (4) Goal + Score
        - Xác định khung thành trái/phải
        - Tăng điểm
        - Reset bóng
    - (5) Hiển thị điểm
        - SDL_ttf (khuyến khích)
        - Hoặc text đơn giản

### Deliverable
- Bóng chạy + bật đúng
- Có điểm số hiển thị

📌 Ăn trọn:
- Tương tác (3đ)
= Điểm số (1đ)

### ⭐ PHẦN BONUS – AI & NGOẠI LỰC
🔹 Ngoại lực (C hoặc B làm)

- Gió

- Đá mạnh → tăng vận tốc

- Random force nhẹ

🔹 Player vs Computer (B làm là hợp nhất)

- AI chỉ cần đuổi theo bóng

- Không cần thông minh


## 🧩 CÁCH GHÉP CODE KHÔNG ĐỤNG NHAU
### File gợi ý
```
src/
 ├── main.cpp
 ├── game.h / game.cpp        (A)
 ├── player.h / player.cpp    (B)
 ├── ball.h / ball.cpp        (C)
 ├── collision.h              (C)
 ├── score.h / score.cpp      (C)
 ```