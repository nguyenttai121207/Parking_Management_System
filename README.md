# 🅿️ ỨNG DỤNG QUẢN LÝ BÃI GIỮ XE THÔNG MINH (PARKING MANAGEMENT SYSTEM)

Dự án phần mềm máy tính (Desktop GUI) được phát triển bằng **C++17**, **Qt6 Widgets**, **SQLite3** và **CMake**, được thiết kế theo đúng chuẩn kiến trúc Hướng Đối Tượng (OOP) và các Mẫu Thiết Kế (Design Patterns).

---

## 1. Tính Năng & Nghiệp Vụ Cốt Lõi

1. **Phân loại 4 dòng phương tiện**:
   - Xe máy xăng
   - Xe máy điện
   - Ô tô xăng
   - Ô tô điện
2. **Quản lý vị trí đỗ (Slot Allocation)**:
   - Bãi đỗ chia 2 phân khu: Khu vực đỗ xe máy (M-01 đến M-20) và Khu vực đỗ ô tô (C-01 đến C-10).
   - Tự động tìm kiếm ô đỗ trống đầu tiên khi check-in, tự động từ chối và cảnh báo lỗi nếu hết chỗ.
3. **Cơ chế tính tiền linh hoạt (Strategy Pattern)**:
   - **Vé theo giờ**: Tính theo thời gian thực từ lúc vào đến lúc ra.
     - *Quy tắc làm tròn*: Phần lẻ $\ge 15$ phút tính thêm 1 giờ trọn vẹn. Tối thiểu 1 giờ.
   - **Vé tháng**: Cho phép đăng ký theo gói 1, 3, 6, 12 tháng. Trong thời hạn hiệu lực, xe vào/ra hoàn toàn miễn phí (0 VNĐ). Tự động fallback sang tính theo giờ nếu vé tháng hết hạn.
4. **Quản lý & Cấu hình bảng giá động (Admin Screen)**:
   - Toàn bộ giá giờ và giá tháng được lưu trữ trong bảng SQLite `pricing_config`, có thể điều chỉnh qua giao diện Admin mà không cần sửa code.
5. **Dashboard thời gian thực & Tra cứu lịch sử**:
   - Thống kê tỷ lệ lấp đầy xe máy, ô tô, tổng số xe đang đậu, doanh thu trong ngày.
   - Bảng theo dõi trực quan các xe đang trong bãi (nhấp đúp chuột để thanh toán check-out nhanh).
   - Tra cứu lịch sử xe ra vào theo biển số.

---

## 2. Thể Hiện Các Nguyên Lý OOP & Design Patterns

| Nguyên Lý / Pattern | File mã nguồn | Giải thích & Minh họa trong code |
| :--- | :--- | :--- |
| **Abstraction (Tính trừu tượng)** | `src/models/Vehicle.h` | Lớp `Vehicle` khai báo các phương thức thuần ảo (`pure virtual`): `getType()`, `getDisplayName()`, `getRequiredSlotType()`. Lớp cha không thể khởi tạo trực tiếp mà đóng vai trò là bản thiết kế giao diện chung. |
| **Inheritance (Tính kế thừa)** | `src/models/Vehicle.h`, `Vehicle.cpp` | `MotorbikeGas`, `MotorbikeElectric`, `CarGas`, `CarElectric` kế thừa từ `Vehicle`, tái sử dụng thuộc tính biển số và định nghĩa chi tiết hành vi của riêng mình. |
| **Polymorphism (Tính đa hình)** | `src/services/ParkingManager.cpp` | Gọi hàm ảo `vehicle->validateLicensePlate()` và `vehicle->getRequiredSlotType()` thông qua con trỏ lớp cơ sở `std::unique_ptr<Vehicle>`. Không cần viết `if (type == ...)` rải rác. |
| **Encapsulation (Tính đóng gói)** | Tất cả các Model trong `src/models/` | Tất cả thuộc tính (như `m_licensePlate`, `m_slotId`, `m_totalFee`) đều là `private` hoặc `protected`, chỉ được truy xuất qua getter/setter và các hàm nghiệp vụ có validation. |
| **Virtual Destructor** | `src/models/Vehicle.h`, `IPricingStrategy.h` | Khai báo `virtual ~Vehicle() = default;` bắt buộc trong C++ để khi hủy đối tượng đa hình qua con trỏ lớp cha, destructor của lớp con tương ứng sẽ được gọi sạch sẽ, tránh memory leak. |
| **Strategy Pattern** | `src/strategies/` | Tách rời thuật toán tính tiền ra khỏi đối tượng: `IPricingStrategy` làm interface, `HourlyPricingStrategy` (tính giờ) và `MonthlyPassStrategy` (vé tháng) là các concrete strategy có thể hoán đổi linh hoạt lúc runtime. |
| **Repository Pattern** | `src/db/ParkingRepository.h/.cpp` | Tách biệt toàn bộ truy vấn SQL ra khỏi code nghiệp vụ. Service và UI chỉ làm việc với Model C++, không dính dáng đến câu lệnh SQL. |
| **Singleton Pattern** | `src/db/DatabaseManager.h/.cpp` | Đảm bảo chỉ có duy nhất 1 kết nối SQLite mở trong toàn bộ vòng đời của ứng dụng thông qua `DatabaseManager::instance()`. |
| **Factory Pattern** | `VehicleFactory::createVehicle(...)` | Đóng gói logic tạo đối tượng phương tiện con phù hợp dựa trên `VehicleType`. |

---

## 3. Cấu Trúc Thư Mục Dự Án

```
D:\ParkingManagementSystem/
├── CMakeLists.txt              # Cấu hình biên dịch dự án
├── README.md                   # Hướng dẫn chi tiết
├── resources/
│   ├── app.qrc                 # Quản lý tài nguyên Qt
│   └── styles/
│       └── dark_theme.qss      # Giao diện hiện đại phong cách Dark Theme
└── src/
    ├── main.cpp                # Điểm khởi chạy ứng dụng
    ├── models/                 # Tầng thực thể dữ liệu (OOP Models)
    │   ├── VehicleType.h       # Enum loại xe & vị trí đỗ
    │   ├── Vehicle.h           # Lớp trừu tượng và 4 lớp xe con
    │   ├── Vehicle.cpp
    │   ├── ParkingSlot.h       # Vị trí đỗ xe
    │   ├── Ticket.h            # Vé lượt xe vào/ra
    │   ├── MonthlySubscription.h # Gói vé tháng
    │   └── PricingConfig.h     # Cấu hình bảng giá
    ├── strategies/             # Strategy Pattern (Tính phí)
    │   ├── IPricingStrategy.h  # Giao diện chiến lược tính giá
    │   ├── HourlyPricingStrategy.h
    │   ├── HourlyPricingStrategy.cpp
    │   ├── MonthlyPassStrategy.h
    │   └── MonthlyPassStrategy.cpp
    ├── db/                     # Tầng CSDL SQLite (Repository & Singleton)
    │   ├── DatabaseManager.h   # Singleton quản lý kết nối SQLite
    │   ├── DatabaseManager.cpp
    │   ├── ParkingRepository.h # Thao tác CRUD dữ liệu
    │   └── ParkingRepository.cpp
    ├── services/               # Tầng nghiệp vụ (Business Logic)
    │   ├── PricingService.h    # Điều phối tính tiền đa hình
    │   ├── PricingService.cpp
    │   ├── ParkingManager.h    # Facade trung tâm điều phối toàn bãi xe
    │   └── ParkingManager.cpp
    └── ui/                     # Tầng giao diện người dùng (Qt Widgets)
        ├── MainWindow.h        # Cửa sổ Dashboard chính
        ├── MainWindow.cpp
        ├── CheckInDialog.h     # Hộp thoại quẹt xe vào
        ├── CheckInDialog.cpp
        ├── CheckOutDialog.h    # Hộp thoại tính tiền xe ra
        ├── CheckOutDialog.cpp
        ├── SubscriptionDialog.h# Quản lý đăng ký vé tháng
        ├── SubscriptionDialog.cpp
        ├── PricingAdminDialog.h# Quản trị viên cập nhật giá
        ├── PricingAdminDialog.cpp
        ├── HistoryWidget.h     # Tra cứu lịch sử xe
        └── HistoryWidget.cpp
```

---

## 4. Hướng Dẫn Biên Dịch & Chạy Ứng Dụng

### Cách 1: Dùng Qt Creator (Khuyên Dùng cho Sinh Viên)
1. Mở phần mềm **Qt Creator**.
2. Chọn **Open File or Project...** (`Ctrl + O`).
3. Điều hướng đến thư mục `D:\ParkingManagementSystem` và chọn tệp `CMakeLists.txt`.
4. Chọn Kit Qt6 tương ứng (ví dụ: `Desktop Qt 6.x.x MinGW 64-bit` hoặc `MSVC`).
5. Bấm nút **Run** (biểu tượng tam giác xanh lá hoặc `Ctrl + R`) để tự động cấu hình, biên dịch và chạy ứng dụng.

### Cách 2: Dùng VS Code
1. Mở thư mục `D:\ParkingManagementSystem` trong VS Code.
2. Cài đặt tiện ích mở rộng **CMake Tools** và **C/C++**.
3. Chọn Kit Qt6 trên thanh trạng thái dưới cùng.
4. Bấm `F7` để Build, bấm `Shift + F5` để Run.

### Cách 3: Biên dịch từ Dòng lệnh (Terminal / PowerShell)
Yêu cầu đã cài đặt CMake và Qt6 (đã set biến môi trường `CMAKE_PREFIX_PATH` trỏ tới thư mục Qt6):
```powershell
# Di chuyển vào thư mục dự án
cd D:\ParkingManagementSystem

# Tạo thư mục build và cấu hình
cmake -B build -S . -DCMAKE_PREFIX_PATH="C:\Qt\6.x.x\mingw_64"

# Biên dịch dự án
cmake --build build --config Release

# Chạy ứng dụng
.\build\ParkingManagementSystem.exe
```

---

## 5. Dữ Liệu Khởi Tạo Mặc Định (Database Seeding)
Khi khởi chạy lần đầu tiên, hệ thống sẽ tự động tạo file `parking_system.db` và nạp sẵn dữ liệu ban đầu:
- **Bảng giá mặc định**:
  - Xe máy xăng: 5.000 VNĐ/giờ | 100.000 VNĐ/tháng
  - Xe máy điện: 4.000 VNĐ/giờ | 80.000 VNĐ/tháng (Chính sách ưu đãi năng lượng sạch)
  - Ô tô xăng: 25.000 VNĐ/giờ | 1.200.000 VNĐ/tháng
  - Ô tô điện: 20.000 VNĐ/giờ | 1.000.000 VNĐ/tháng
- **Vị trí ô đỗ**:
  - 20 chỗ xe máy (`M-01` $\to$ `M-20`)
  - 10 chỗ ô tô (`C-01` $\to$ `C-10`)
