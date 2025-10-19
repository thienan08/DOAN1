// Cấu hình Firebase
const firebaseConfig = {
    apiKey: "AIzaSyCDfdFWiQWFeVaU8F81tVmOTRppC_7996U",
    authDomain: "vutienluc-f6439.firebaseapp.com",
    databaseURL: "https://vutienluc-f6439-default-rtdb.firebaseio.com",
    projectId: "vutienluc-f6439",
    storageBucket: "vutienluc-f6439.firebasestorage.app",
    messagingSenderId: "655894392844",
    appId: "1:655894392844:web:f87e761d8080ddb0ce3e29"
};

// Khởi tạo Firebase
firebase.initializeApp(firebaseConfig);
const database = firebase.database();

// Các ngưỡng cảnh báo mặc định
const DEFAULT_THRESHOLDS = {
    gas: 1,             // 1 = phát hiện gas, 0 = không phát hiện
    temperature: 40,    // °C - ngưỡng cảnh báo nhiệt độ cao
    humidity: {
        high: 80        // % - ngưỡng cảnh báo độ ẩm cao
    },
    flame: 1            // 1 = phát hiện lửa, 0 = không phát hiện
};

// Đối tượng lưu trữ trạng thái ngưỡng cho từng phòng
const THRESHOLDS = {
    room1: {...DEFAULT_THRESHOLDS},
    room2: {...DEFAULT_THRESHOLDS}
};

// Biến để theo dõi trạng thái bật/tắt buzzer và LED
let buzzerEnabled = true;
let ledEnabled = true;

// Lấy các phần tử DOM cho toàn bộ ứng dụng
const connectionStatus = document.getElementById('connectionStatus');
const alertToggle = document.getElementById('alertToggle');
const ledToggle = document.getElementById('ledToggle');

// Lấy các phần tử DOM cho Phòng 1
const gasValue1 = document.getElementById('gasValue1');
const gasStatus1 = document.getElementById('gasStatus1');
const gasCard1 = document.getElementById('gasCard1');
const gasAlert1 = document.getElementById('gasAlert1');
const gasAlertText1 = document.getElementById('gasAlertText1');

const temperatureValue1 = document.getElementById('temperatureValue1');
const tempStatus1 = document.getElementById('tempStatus1');
const tempCard1 = document.getElementById('tempCard1');
const tempAlert1 = document.getElementById('tempAlert1');
const tempAlertText1 = document.getElementById('tempAlertText1');

const humidityValue1 = document.getElementById('humidityValue1');
const humidityStatus1 = document.getElementById('humidityStatus1');
const humidityCard1 = document.getElementById('humidityCard1');
const humidityAlert1 = document.getElementById('humidityAlert1');
const humidityAlertText1 = document.getElementById('humidityAlertText1');

const flameValue1 = document.getElementById('flameValue1');
const flameStatus1 = document.getElementById('flameStatus1');
const flameCard1 = document.getElementById('flameCard1');
const flameAlert1 = document.getElementById('flameAlert1');
const flameAlertText1 = document.getElementById('flameAlertText1');

const settingsBtn1 = document.getElementById('settingsBtn1');
const settingsPanel1 = document.getElementById('settingsPanel1');
const saveSettings1 = document.getElementById('saveSettings1');
const gasThreshold1 = document.getElementById('gasThreshold1');
const tempThreshold1 = document.getElementById('tempThreshold1');
const humidityHighThreshold1 = document.getElementById('humidityHighThreshold1');

// Lấy các phần tử DOM cho Phòng 2
const gasValue2 = document.getElementById('gasValue2');
const gasStatus2 = document.getElementById('gasStatus2');
const gasCard2 = document.getElementById('gasCard2');
const gasAlert2 = document.getElementById('gasAlert2');
const gasAlertText2 = document.getElementById('gasAlertText2');

const temperatureValue2 = document.getElementById('temperatureValue2');
const tempStatus2 = document.getElementById('tempStatus2');
const tempCard2 = document.getElementById('tempCard2');
const tempAlert2 = document.getElementById('tempAlert2');
const tempAlertText2 = document.getElementById('tempAlertText2');

const humidityValue2 = document.getElementById('humidityValue2');
const humidityStatus2 = document.getElementById('humidityStatus2');
const humidityCard2 = document.getElementById('humidityCard2');
const humidityAlert2 = document.getElementById('humidityAlert2');
const humidityAlertText2 = document.getElementById('humidityAlertText2');

const flameValue2 = document.getElementById('flameValue2');
const flameStatus2 = document.getElementById('flameStatus2');
const flameCard2 = document.getElementById('flameCard2');
const flameAlert2 = document.getElementById('flameAlert2');
const flameAlertText2 = document.getElementById('flameAlertText2');

const settingsBtn2 = document.getElementById('settingsBtn2');
const settingsPanel2 = document.getElementById('settingsPanel2');
const saveSettings2 = document.getElementById('saveSettings2');
const gasThreshold2 = document.getElementById('gasThreshold2');
const tempThreshold2 = document.getElementById('tempThreshold2');
const humidityHighThreshold2 = document.getElementById('humidityHighThreshold2');

// Đọc ngưỡng cảnh báo từ Firebase khi khởi động
function loadThresholdsFromFirebase() {
    database.ref('thresholds').once('value', (snapshot) => {
        if (snapshot.exists()) {
            const fbThresholds = snapshot.val();
            
            // Cập nhật ngưỡng từ Firebase cho phòng 1
            if (fbThresholds.room1) {
                if (fbThresholds.room1.temperature) {
                    THRESHOLDS.room1.temperature = fbThresholds.room1.temperature;
                }
                if (fbThresholds.room1.humidity && fbThresholds.room1.humidity.high) {
                    THRESHOLDS.room1.humidity.high = fbThresholds.room1.humidity.high;
                }
            }
            
            // Cập nhật ngưỡng từ Firebase cho phòng 2
            if (fbThresholds.room2) {
                if (fbThresholds.room2.temperature) {
                    THRESHOLDS.room2.temperature = fbThresholds.room2.temperature;
                }
                if (fbThresholds.room2.humidity && fbThresholds.room2.humidity.high) {
                    THRESHOLDS.room2.humidity.high = fbThresholds.room2.humidity.high;
                }
            }
            
            // Cập nhật giao diện với giá trị từ Firebase
            initializeThresholdValues();
        } else {
            // Nếu chưa có dữ liệu ngưỡng, tạo mới với giá trị mặc định
            database.ref('thresholds').set(THRESHOLDS);
        }
    });
}

// Khởi tạo thiết lập ngưỡng mặc định cho giao diện
function initializeThresholdValues() {
    // Phòng 1
    tempThreshold1.value = THRESHOLDS.room1.temperature;
    humidityHighThreshold1.value = THRESHOLDS.room1.humidity.high;
    
    // Phòng 2
    tempThreshold2.value = THRESHOLDS.room2.temperature;
    humidityHighThreshold2.value = THRESHOLDS.room2.humidity.high;
}

// Xử lý các sự kiện cài đặt
function setupSettingsControls() {
    // Phòng 1
    settingsBtn1.addEventListener('click', () => {
        settingsPanel1.classList.toggle('active');
    });
    
    saveSettings1.addEventListener('click', () => {
        // Cập nhật ngưỡng cho Phòng 1
        THRESHOLDS.room1.temperature = parseInt(tempThreshold1.value);
        THRESHOLDS.room1.humidity.high = parseInt(humidityHighThreshold1.value);
        
        // Lưu ngưỡng vào Firebase
        database.ref('thresholds/room1').update({
            temperature: THRESHOLDS.room1.temperature,
            humidity: {
                high: THRESHOLDS.room1.humidity.high
            }
        });
        
        // Cập nhật kiểm tra cảnh báo
        processRoom1Data();
        
        // Ẩn bảng cài đặt
        settingsPanel1.classList.remove('active');
    });
    
    // Phòng 2
    settingsBtn2.addEventListener('click', () => {
        settingsPanel2.classList.toggle('active');
    });
    
    saveSettings2.addEventListener('click', () => {
        // Cập nhật ngưỡng cho Phòng 2
        THRESHOLDS.room2.temperature = parseInt(tempThreshold2.value);
        THRESHOLDS.room2.humidity.high = parseInt(humidityHighThreshold2.value);
        
        // Lưu ngưỡng vào Firebase
        database.ref('thresholds/room2').update({
            temperature: THRESHOLDS.room2.temperature,
            humidity: {
                high: THRESHOLDS.room2.humidity.high
            }
        });
        
        // Cập nhật kiểm tra cảnh báo
        processRoom2Data();
        
        // Ẩn bảng cài đặt
        settingsPanel2.classList.remove('active');
    });
}

// Xử lý nút bật/tắt buzzer và LED
function setupAlertControls() {
    // Lắng nghe liên tục trạng thái buzzer từ Firebase
    database.ref('controls/buzzer').on('value', (snapshot) => {
        if (snapshot.exists()) {
            buzzerEnabled = snapshot.val() === 1;
            alertToggle.checked = buzzerEnabled;
        } else {
            // Nếu chưa có dữ liệu, tạo mới với giá trị mặc định là bật
            database.ref('controls/buzzer').set(1);
            buzzerEnabled = true;
            alertToggle.checked = true;
        }
    });
    
    // Lắng nghe liên tục trạng thái LED từ Firebase
    database.ref('controls/led').on('value', (snapshot) => {
        if (snapshot.exists()) {
            ledEnabled = snapshot.val() === 1;
            ledToggle.checked = ledEnabled;
        } else {
            // Nếu chưa có dữ liệu, tạo mới với giá trị mặc định là bật
            database.ref('controls/led').set(1);
            ledEnabled = true;
            ledToggle.checked = true;
        }
    });
    
    alertToggle.addEventListener('change', () => {
        buzzerEnabled = alertToggle.checked;
        
        // Gửi trạng thái buzzer đến Firebase
        database.ref('controls/buzzer').set(buzzerEnabled ? 1 : 0);
    });
    
    ledToggle.addEventListener('change', () => {
        ledEnabled = ledToggle.checked;
        
        // Gửi trạng thái LED đến Firebase
        database.ref('controls/led').set(ledEnabled ? 1 : 0);
    });
}

// Kiểm tra kết nối
database.ref('.info/connected').on('value', (snapshot) => {
    if (snapshot.val() === true) {
        connectionStatus.textContent = 'Đã kết nối';
        connectionStatus.classList.add('connected');
        connectionStatus.classList.remove('disconnected');
    } else {
        connectionStatus.textContent = 'Mất kết nối';
        connectionStatus.classList.add('disconnected');
        connectionStatus.classList.remove('connected');
    }
});

// Tham chiếu đến các node dữ liệu cho Phòng 1
const room1Ref = database.ref('rooms/room1');
room1Ref.on('value', (snapshot) => {
    const data = snapshot.val() || {};
    
    // Cập nhật giá trị cho từng cảm biến
    if (data.gas !== undefined) {
        // Gas là giá trị nhị phân (0 hoặc 1)
        const gasDetected = data.gas === 1;
        gasValue1.textContent = gasDetected ? 'Phát hiện' : 'Không phát hiện';
    } else {
        gasValue1.textContent = '--';
    }
    
    if (data.temperature !== undefined) {
        temperatureValue1.textContent = data.temperature + ' °C';
    } else {
        temperatureValue1.textContent = '-- °C';
    }
    
    if (data.humidity !== undefined) {
        humidityValue1.textContent = data.humidity + ' %';
    } else {
        humidityValue1.textContent = '-- %';
    }
    
    if (data.flame !== undefined) {
        flameValue1.textContent = data.flame === 1 ? 'Phát hiện' : 'Không phát hiện';
    } else {
        flameValue1.textContent = '--';
    }
    
    // Kiểm tra cảnh báo
    processRoom1Data();
});

// Tham chiếu đến các node dữ liệu cho Phòng 2
const room2Ref = database.ref('rooms/room2');
room2Ref.on('value', (snapshot) => {
    const data = snapshot.val() || {};
    
    // Cập nhật giá trị cho từng cảm biến
    if (data.gas !== undefined) {
        // Gas là giá trị nhị phân (0 hoặc 1)
        const gasDetected = data.gas === 1;
        gasValue2.textContent = gasDetected ? 'Phát hiện' : 'Không phát hiện';
    } else {
        gasValue2.textContent = '--';
    }
    
    if (data.temperature !== undefined) {
        temperatureValue2.textContent = data.temperature + ' °C';
    } else {
        temperatureValue2.textContent = '-- °C';
    }
    
    if (data.humidity !== undefined) {
        humidityValue2.textContent = data.humidity + ' %';
    } else {
        humidityValue2.textContent = '-- %';
    }
    
    if (data.flame !== undefined) {
        flameValue2.textContent = data.flame === 1 ? 'Phát hiện' : 'Không phát hiện';
    } else {
        flameValue2.textContent = '--';
    }
    
    // Kiểm tra cảnh báo
    processRoom2Data();
});

// Xử lý dữ liệu và hiển thị cảnh báo cho Phòng 1
function processRoom1Data() {
    // Thay đổi cách kiểm tra giá trị gas
    const gasDetected = gasValue1.textContent === 'Phát hiện';
    const tempValue = parseFloat(temperatureValue1.textContent);
    const humidityValue = parseFloat(humidityValue1.textContent);
    const flameDetected = flameValue1.textContent === 'Phát hiện';
    let hasAlert = false;
    
    // Kiểm tra Gas
    if (gasValue1.textContent !== '--') {
        if (gasDetected) {
            gasCard1.classList.add('warning');
            gasCard1.classList.remove('normal');
            gasStatus1.textContent = 'CẢNH BÁO: Phát hiện khí gas!';
            gasAlert1.classList.add('active');
            gasAlertText1.textContent = 'Phát hiện khí gas! Kiểm tra ngay!';
            hasAlert = true;
        } else {
            gasCard1.classList.add('normal');
            gasCard1.classList.remove('warning');
            gasStatus1.textContent = 'Bình thường';
            gasAlert1.classList.remove('active');
        }
    } else {
        gasStatus1.textContent = 'Không có dữ liệu';
        gasCard1.classList.remove('warning', 'normal');
        gasAlert1.classList.remove('active');
    }
    
    // Kiểm tra nhiệt độ
    if (!isNaN(tempValue)) {
        if (tempValue > THRESHOLDS.room1.temperature) {
            tempCard1.classList.add('warning');
            tempCard1.classList.remove('normal');
            tempStatus1.textContent = 'CẢNH BÁO: Nhiệt độ cao!';
            tempAlert1.classList.add('active');
            tempAlertText1.textContent = `Nhiệt độ vượt ngưỡng (${THRESHOLDS.room1.temperature} °C)`;
            hasAlert = true;
        } else {
            tempCard1.classList.add('normal');
            tempCard1.classList.remove('warning');
            tempStatus1.textContent = 'Bình thường';
            tempAlert1.classList.remove('active');
        }
    } else {
        tempStatus1.textContent = 'Không có dữ liệu';
        tempCard1.classList.remove('warning', 'normal');
        tempAlert1.classList.remove('active');
    }
    
    // Kiểm tra độ ẩm
    if (!isNaN(humidityValue)) {
        if (humidityValue > THRESHOLDS.room1.humidity.high) {
            humidityCard1.classList.add('warning');
            humidityCard1.classList.remove('normal');
            humidityStatus1.textContent = 'CẢNH BÁO: Độ ẩm quá cao!';
            humidityAlert1.classList.add('active');
            humidityAlertText1.textContent = `Độ ẩm vượt ngưỡng cao (${THRESHOLDS.room1.humidity.high}%)`;
            hasAlert = true;
        } else {
            humidityCard1.classList.add('normal');
            humidityCard1.classList.remove('warning');
            humidityStatus1.textContent = 'Bình thường';
            humidityAlert1.classList.remove('active');
        }
    } else {
        humidityStatus1.textContent = 'Không có dữ liệu';
        humidityCard1.classList.remove('warning', 'normal');
        humidityAlert1.classList.remove('active');
    }
    
    // Kiểm tra phát hiện lửa
    if (flameValue1.textContent !== '--') {
        if (flameDetected) {
            flameCard1.classList.add('warning');
            flameCard1.classList.remove('normal');
            flameStatus1.textContent = 'CẢNH BÁO: Phát hiện lửa!';
            flameAlert1.classList.add('active');
            flameAlertText1.textContent = 'Phát hiện lửa! Kiểm tra ngay!';
            hasAlert = true;
        } else {
            flameCard1.classList.add('normal');
            flameCard1.classList.remove('warning');
            flameStatus1.textContent = 'Bình thường';
            flameAlert1.classList.remove('active');
        }
    } else {
        flameStatus1.textContent = 'Không có dữ liệu';
        flameCard1.classList.remove('warning', 'normal');
        flameAlert1.classList.remove('active');
    }
    
    // Cập nhật trạng thái cảnh báo cho phòng 1
    database.ref('alerts/room1').set(hasAlert ? 1 : 0);
}

// Xử lý dữ liệu và hiển thị cảnh báo cho Phòng 2
function processRoom2Data() {
    // Thay đổi cách kiểm tra giá trị gas
    const gasDetected = gasValue2.textContent === 'Phát hiện';
    const tempValue = parseFloat(temperatureValue2.textContent);
    const humidityValue = parseFloat(humidityValue2.textContent);
    const flameDetected = flameValue2.textContent === 'Phát hiện';
    let hasAlert = false;
    
    // Kiểm tra Gas
    if (gasValue2.textContent !== '--') {
        if (gasDetected) {
            gasCard2.classList.add('warning');
            gasCard2.classList.remove('normal');
            gasStatus2.textContent = 'CẢNH BÁO: Phát hiện khí gas!';
            gasAlert2.classList.add('active');
            gasAlertText2.textContent = 'Phát hiện khí gas! Kiểm tra ngay!';
            hasAlert = true;
        } else {
            gasCard2.classList.add('normal');
            gasCard2.classList.remove('warning');
            gasStatus2.textContent = 'Bình thường';
            gasAlert2.classList.remove('active');
        }
    } else {
        gasStatus2.textContent = 'Không có dữ liệu';
        gasCard2.classList.remove('warning', 'normal');
        gasAlert2.classList.remove('active');
    }
    
    // Kiểm tra nhiệt độ
    if (!isNaN(tempValue)) {
        if (tempValue > THRESHOLDS.room2.temperature) {
            tempCard2.classList.add('warning');
            tempCard2.classList.remove('normal');
            tempStatus2.textContent = 'CẢNH BÁO: Nhiệt độ cao!';
            tempAlert2.classList.add('active');
            tempAlertText2.textContent = `Nhiệt độ vượt ngưỡng (${THRESHOLDS.room2.temperature} °C)`;
            hasAlert = true;
        } else {
            tempCard2.classList.add('normal');
            tempCard2.classList.remove('warning');
            tempStatus2.textContent = 'Bình thường';
            tempAlert2.classList.remove('active');
        }
    } else {
        tempStatus2.textContent = 'Không có dữ liệu';
        tempCard2.classList.remove('warning', 'normal');
        tempAlert2.classList.remove('active');
    }
    
    // Kiểm tra độ ẩm
    if (!isNaN(humidityValue)) {
        if (humidityValue > THRESHOLDS.room2.humidity.high) {
            humidityCard2.classList.add('warning');
            humidityCard2.classList.remove('normal');
            humidityStatus2.textContent = 'CẢNH BÁO: Độ ẩm quá cao!';
            humidityAlert2.classList.add('active');
            humidityAlertText2.textContent = `Độ ẩm vượt ngưỡng cao (${THRESHOLDS.room2.humidity.high}%)`;
            hasAlert = true;
        } else {
            humidityCard2.classList.add('normal');
            humidityCard2.classList.remove('warning');
            humidityStatus2.textContent = 'Bình thường';
            humidityAlert2.classList.remove('active');
        }
    } else {
        humidityStatus2.textContent = 'Không có dữ liệu';
        humidityCard2.classList.remove('warning', 'normal');
        humidityAlert2.classList.remove('active');
    }
    
    // Kiểm tra phát hiện lửa
    if (flameValue2.textContent !== '--') {
        if (flameDetected) {
            flameCard2.classList.add('warning');
            flameCard2.classList.remove('normal');
            flameStatus2.textContent = 'CẢNH BÁO: Phát hiện lửa!';
            flameAlert2.classList.add('active');
            flameAlertText2.textContent = 'Phát hiện lửa! Kiểm tra ngay!';
            hasAlert = true;
        } else {
            flameCard2.classList.add('normal');
            flameCard2.classList.remove('warning');
            flameStatus2.textContent = 'Bình thường';
            flameAlert2.classList.remove('active');
        }
    } else {
        flameStatus2.textContent = 'Không có dữ liệu';
        flameCard2.classList.remove('warning', 'normal');
        flameAlert2.classList.remove('active');
    }
    
    // Cập nhật trạng thái cảnh báo cho phòng 2
    database.ref('alerts/room2').set(hasAlert ? 1 : 0);
}

// Khởi tạo ứng dụng
function initializeApp() {
    // Đọc ngưỡng từ Firebase trước
    loadThresholdsFromFirebase();
    
    // Thiết lập điều khiển cài đặt
    setupSettingsControls();
    
    // Thiết lập nút bật/tắt buzzer và LED
    setupAlertControls();
}

// Khởi động ứng dụng
initializeApp(); 
