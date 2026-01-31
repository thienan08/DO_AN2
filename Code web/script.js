import { initializeApp } from "https://www.gstatic.com/firebasejs/9.23.0/firebase-app.js";
import {
  getDatabase,
  ref,
  query,
  orderByKey,
  limitToLast,
  onValue
} from "https://www.gstatic.com/firebasejs/9.23.0/firebase-database.js";

// ==================== CẤU HÌNH FIREBASE ====================
const firebaseConfig = {
    apiKey: "",
    authDomain: "power-monitoring-c523c.firebaseapp.com",
    databaseURL: "https://power-monitoring-c523c-default-rtdb.asia-southeast1.firebasedatabase.app",
    projectId: "power-monitoring-c523c",
    storageBucket: "power-monitoring-c523c.firebasestorage.app",
    messagingSenderId: "797293093488",
    appId: "1:797293093488:web:be519fec0b9820a7630ab5",
    measurementId: "G-6KN4LCX5V6"
};
const app = initializeApp(firebaseConfig);
const db = getDatabase(app);

// ==================== DỮ LIỆU ====================
let settings = {
    room1PowerThreshold: 150,
    room2PowerThreshold: 150,
    room1CostThreshold: 150 * 3500,
    room2CostThreshold: 150 * 3500,
};

const roomData = { C000001: {}, C000002: {} };
let paymentData = [];
let energy1 = 0;
let energy2 = 0;
let Voltage1 = 0
let Voltage2 = 0
let electricityPrice = 3500

// ==================== KHỞI TẠO ====================
document.addEventListener('DOMContentLoaded', () => {
    initDateTime();
    initNavigation();
    initModals();
    loadSettings();
    loadPaymentData();
    renderPaymentTable();
});

// ==================== FIREBASE ====================
onValue(ref(db, "UnitPrice"), snapshot => {
    if (snapshot.exists()) {
        electricityPrice = snapshot.val()
        console.log(electricityPrice)
        updateDashboard()
    }
})

onValue(ref(db, "C000001/Voltage"), snapshot => {
    if (snapshot.exists()) {
        Voltage1 = snapshot.val()
        updateDashboard()
    }
})

const dataRef1 = ref(db, "C000001/Data");
const q1 = query(
    dataRef1,
    orderByKey(),
    limitToLast(1)
);

onValue(q1, (snapshot) => {
  if (!snapshot.exists()) return;

  snapshot.forEach((lastDaySnap) => {

    let lastHourKey = null;
    let lastHourValue = null;

    lastDaySnap.forEach((hourSnap) => {
      lastHourKey = hourSnap.key;
      lastHourValue = hourSnap.val();
    });

    if (lastHourValue !== null) {
      energy1 = lastHourValue;
      console.log("Latest energy 1:", energy1);
      updateDashboard();
    }
  });
});

onValue(ref(db, "C000002/Voltage"), snapshot => {
    if (snapshot.exists()) {
        Voltage2 = snapshot.val();
        updateDashboard();
    }
})

const dataRef2 = ref(db, "C000002/Data");
const q2 = query(
    dataRef2,
    orderByKey(),
    limitToLast(1)
);

onValue(q2, (snapshot) => {
  if (!snapshot.exists()) return;

  snapshot.forEach((lastDaySnap) => {

    let lastHourKey = null;
    let lastHourValue = null;

    lastDaySnap.forEach((hourSnap) => {
      lastHourKey = hourSnap.key;
      lastHourValue = hourSnap.val();
    });

    if (lastHourValue !== null) {
      energy2 = lastHourValue;
      console.log("Latest energy 2:", energy2);
      updateDashboard();
    }
  });
});

// ==================== THỜI GIAN ====================
function initDateTime() {
    updateDateTime();
    setInterval(updateDateTime, 1000);
}
function updateDateTime() {
    const now = new Date();
    document.getElementById('currentDate').textContent =
        now.toLocaleDateString('vi-VN', { weekday: 'long', year: 'numeric', month: 'long', day: 'numeric' });
    document.getElementById('currentTime').textContent =
        now.toLocaleTimeString('vi-VN', { hour: '2-digit', minute: '2-digit', second: '2-digit' });
}

// ==================== ĐIỀU HƯỚNG ====================
function initNavigation() {
    const navItems = document.querySelectorAll('.nav-item');
    const sections = {
        dashboardBtn: 'dashboardSection',
        reportBtn: 'reportSection',
        paymentBtn: 'paymentSection'
    };
    navItems.forEach(item => {
        item.addEventListener('click', function () {
            navItems.forEach(nav => nav.classList.remove('active'));
            this.classList.add('active');
            document.querySelectorAll('.content-section').forEach(s => s.classList.remove('active'));
            const sectionId = sections[this.id];
            if (sectionId) document.getElementById(sectionId).classList.add('active');
        });
    });
}

// ==================== DASHBOARD ====================
function updateDashboard() {
    document.getElementById('room1Voltage').textContent = Voltage1 + " V";
    document.getElementById('room1Energy').textContent = energy1 + ' kWh';
    document.getElementById('room1Cost').textContent = parseInt(electricityPrice * energy1) + " VNĐ";

    document.getElementById('room2Voltage').textContent = Voltage2 + " V";
    document.getElementById('room2Energy').textContent = energy2 + " kWh";
    document.getElementById('room2Cost').textContent = parseInt(electricityPrice * energy2) + " VNĐ";

    updateAlerts();
}

// ==================== TRẠNG THÁI CẢNH BÁO ====================
function updateAlerts() {
    const warn1 = roomData.room101.Warn1 ?? 0;
    const energy1 = roomData.room101.Energy1 ?? 0;
    const threshold1 = roomData.room101.Threshold1 ?? settings.room1PowerThreshold;

    const warn2 = roomData.room102.Warn2 ?? 0;
    const energy2 = roomData.room102.Energy2 ?? 0;
    const threshold2 = roomData.room102.Threshold2 ?? settings.room2PowerThreshold;

    const r1Alert = document.getElementById('room1Alert');
    const r1Msg = document.getElementById('room1AlertMsg');
    const r2Alert = document.getElementById('room2Alert');
    const r2Msg = document.getElementById('room2AlertMsg');

    if (warn1 === 1 || energy1 > threshold1) {
        r1Alert.className = 'param-card alert-card';
        r1Alert.querySelector('.param-icon').textContent = '⚠️';
        r1Msg.textContent = `Vượt ngưỡng ${(threshold1).toFixed(1)} kWh`;
    } else {
        r1Alert.className = 'param-card alert-card success';
        r1Alert.querySelector('.param-icon').textContent = '✅';
        r1Msg.textContent = 'Bình thường';
    }

    if (warn2 === 1 || energy2 > threshold2) {
        r2Alert.className = 'param-card alert-card';
        r2Alert.querySelector('.param-icon').textContent = '⚠️';
        r2Msg.textContent = `Vượt ngưỡng ${(threshold2).toFixed(1)} kWh`;
    } else {
        r2Alert.className = 'param-card alert-card success';
        r2Alert.querySelector('.param-icon').textContent = '✅';
        r2Msg.textContent = 'Bình thường';
    }
}

// ==================== BIỂU ĐỒ ====================
function getLast8MonthsData(dataSnapshot, unitPrice = 2000) {
    if (!dataSnapshot || typeof dataSnapshot !== 'object') {
        return { months: [], energy: [], cost: [] };
    }

    const dayEndValues = [];
    for (const [dayKey, hoursObj] of Object.entries(dataSnapshot)) {
        if (!hoursObj || typeof hoursObj !== 'object') continue;

        const hourValues = Object.values(hoursObj)
            .filter(v => typeof v === 'number')
            .map(Number);

        if (hourValues.length === 0) continue;

        const lastValue = Math.max(...hourValues);
        dayEndValues.push({ dayKey, value: lastValue });
    }

    if (dayEndValues.length === 0) return { months: [], energy: [], cost: [] };

    const monthMap = {};
    dayEndValues.forEach(({ dayKey, value }) => {
        const monthKey = dayKey.substring(0, 7); // "2025-12"
        if (!monthMap[monthKey] || value > monthMap[monthKey].value) {
            monthMap[monthKey] = { value };
        }
    });

    const sortedMonths = Object.keys(monthMap)
        .sort()
        .map(monthKey => ({
            monthKey,
            value: monthMap[monthKey].value,
            label: `${monthKey.substring(5, 7)}/${monthKey.substring(0, 4)}`
        }));

    if (sortedMonths.length < 2) return { months: [], energy: [], cost: [] };

    const result = [];
    for (let i = 1; i < sortedMonths.length; i++) {
        const prev = sortedMonths[i - 1];
        const curr = sortedMonths[i];
        const usage = curr.value - prev.value;
        if (usage >= 0) {
            result.push({
                label: prev.label,
                energy: parseFloat(usage.toFixed(2)),
                cost: Math.round(usage * unitPrice) 
            });
        }
    }

    // Lấy 8 tháng gần nhất
    const last8 = result.slice(-8);

    // Bù thiếu nếu chưa đủ 8 tháng
    while (last8.length < 8) {
        const prevLabel = last8.length > 0 ? last8[0].label : "--/----";
        last8.unshift({ label: prevLabel, energy: 0, cost: 0 });
    }

    const months = last8.map(item => item.label);
    const energy = last8.map(item => item.energy);
    const cost = last8.map(item => item.cost);

    return { months, energy, cost };
}

let months = [];
let chartData1 = { energy: [], cost: [] }; // B00
let chartData2 = { energy: [], cost: [] }; // B01
let loadedCount = 0;

let powerChart, costChart; // Gộp
let powerB01Chart, costB01Chart; // Tách riêng B01

function updateCharts() {
    if (loadedCount < 2) return;

    const isSeparate = document.getElementById('separateChartsToggle').checked;

    // Cập nhật tiêu đề
    document.getElementById('powerTitle').textContent = isSeparate ? "Phòng B00 - Điện năng" : "So sánh Điện năng";
    document.getElementById('costTitle').textContent = isSeparate ? "Phòng B00 - Chi phí" : "So sánh Chi phí";

    document.getElementById('powerChartB01Container').style.display = isSeparate ? 'block' : 'none';
    document.getElementById('costChartB01Container').style.display = isSeparate ? 'block' : 'none';

    // === BIỂU ĐỒ ĐIỆN NĂNG (BAR)
    if (powerChart) powerChart.destroy();
    const powerCtx = document.getElementById('powerChart').getContext('2d');
    powerChart = new Chart(powerCtx, {
        type: 'bar',
        data: {
            labels: months,
            datasets: isSeparate ? 
                [{
                    label: 'Điện năng (kWh)',
                    data: chartData1.energy,
                    backgroundColor: 'rgba(99, 102, 241, 0.8)',  // Indigo
                    borderColor: '#6366f1',
                    borderWidth: 2,
                    borderRadius: 10,
                    borderSkipped: false,
                }] :
                [
                    {
                        label: 'Phòng B00',
                        data: chartData1.energy,
                        backgroundColor: 'rgba(99, 102, 241, 0.8)',  // Indigo
                        borderColor: '#6366f1',
                        borderWidth: 2,
                        borderRadius: 10,
                    },
                    {
                        label: 'Phòng B01',
                        data: chartData2.energy,
                        backgroundColor: 'rgba(168, 85, 247, 0.8)',   // Purple
                        borderColor: '#a855f7',
                        borderWidth: 2,
                        borderRadius: 10,
                    }
                ]
        },
        options: {
            responsive: true,
            plugins: {
                legend: { position: 'top', labels: { font: { size: 14 } } },
                tooltip: { backgroundColor: 'rgba(0,0,0,0.8)', titleColor: '#fff', bodyColor: '#fff' }
            },
            scales: {
                y: {
                    beginAtZero: true,
                    grid: { color: 'rgba(0,0,0,0.05)' },
                    ticks: { color: '#666' }
                },
                x: { grid: { display: false }, ticks: { maxRotation: 45, color: '#666' } }
            }
        }
    });

    // === BIỂU ĐỒ CHI PHÍ (LINE) 
    if (costChart) costChart.destroy();
    const costCtx = document.getElementById('costChart').getContext('2d');
    costChart = new Chart(costCtx, {
        type: 'line',
        data: {
            labels: months,
            datasets: isSeparate ? 
                [{
                    label: 'Chi phí (VNĐ)',
                    data: chartData1.cost,
                    borderColor: '#f97316',              // Cam sáng
                    backgroundColor: 'rgba(249, 115, 22, 0.15)',
                    tension: 0.4,
                    fill: true,
                    pointBackgroundColor: '#f97316',
                    pointRadius: 5,
                    pointHoverRadius: 8,
                }] :
                [
                    {
                        label: 'Phòng B00',
                        data: chartData1.cost,
                        borderColor: '#f97316',              // Cam sáng
                        backgroundColor: 'rgba(249, 115, 22, 0.15)',
                        tension: 0.4,
                        fill: true,
                        pointBackgroundColor: '#f97316',
                        pointRadius: 5,
                    },
                    {
                        label: 'Phòng B01',
                        data: chartData2.cost,
                        borderColor: '#ef4444',              // Đỏ tươi
                        backgroundColor: 'rgba(239, 68, 68, 0.15)',
                        tension: 0.4,
                        fill: true,
                        pointBackgroundColor: '#ef4444',
                        pointRadius: 5,
                    }
                ]
        },
        options: {
            responsive: true,
            plugins: {
                legend: { position: 'top', labels: { font: { size: 14 } } },
                tooltip: {
                    backgroundColor: 'rgba(0,0,0,0.8)',
                    titleColor: '#fff',
                    bodyColor: '#fff',
                    callbacks: { label: ctx => `${ctx.dataset.label}: ${ctx.parsed.y.toLocaleString('vi-VN')} VNĐ` }
                }
            },
            scales: {
                y: {
                    beginAtZero: true,
                    grid: { color: 'rgba(0,0,0,0.05)' },
                    ticks: { 
                        color: '#666',
                        callback: v => v.toLocaleString('vi-VN')
                    }
                },
                x: { grid: { display: false }, ticks: { maxRotation: 45, color: '#666' } }
            }
        }
    });

    // Biểu đồ B01 khi tách (màu giống B01 gộp)
    if (isSeparate) {
        if (powerB01Chart) powerB01Chart.destroy();
        powerB01Chart = new Chart(document.getElementById('powerChartB01').getContext('2d'), {
            type: 'bar',
            data: { labels: months, datasets: [{ label: 'Điện năng (kWh)', data: chartData2.energy, backgroundColor: 'rgba(168, 85, 247, 0.8)', borderColor: '#a855f7', borderWidth: 2, borderRadius: 10 }] },
            options: { responsive: true, plugins: { legend: { position: 'top' } }, scales: { y: { beginAtZero: true } } }
        });

        if (costB01Chart) costB01Chart.destroy();
        costB01Chart = new Chart(document.getElementById('costChartB01').getContext('2d'), {
            type: 'line',
            data: { labels: months, datasets: [{ label: 'Chi phí (VNĐ)', data: chartData2.cost, borderColor: '#ef4444', backgroundColor: 'rgba(239, 68, 68, 0.15)', fill: true, tension: 0.4, pointBackgroundColor: '#ef4444', pointRadius: 5 }] },
            options: { responsive: true, scales: { y: { beginAtZero: true, ticks: { callback: v => v.toLocaleString('vi-VN') } } } }
        });
    }
}

// Switch event
document.getElementById('separateChartsToggle').addEventListener('change', updateCharts);

// Listener B00 và B01
onValue(ref(db, "C000001/Data"), (snapshot) => {
    if (!snapshot.exists()) return;
    const result = getLast8MonthsData(snapshot.val(), electricityPrice);
    chartData1.energy = result.energy || [];
    chartData1.cost = result.cost || [];
    if (loadedCount === 0) months = result.months || [];
    loadedCount = Math.max(loadedCount, 1);
    updateCharts();
});

onValue(ref(db, "C000002/Data"), (snapshot) => {
    if (!snapshot.exists()) return;
    const result = getLast8MonthsData(snapshot.val(), electricityPrice);
    chartData2.energy = result.energy || [];
    chartData2.cost = result.cost || [];
    if (loadedCount === 0) months = result.months || [];
    loadedCount = 2;
    updateCharts();
});

// ==================== MODALS ====================
function initModals() {
    const settingsBtn = document.getElementById('settingsBtn');
    const settingsModal = document.getElementById('settingsModal');
    const saveSettingsBtn = document.getElementById('saveSettings');
    const addPaymentBtn = document.getElementById('addPaymentBtn');
    const addPaymentModal = document.getElementById('addPaymentModal');
    const savePaymentBtn = document.getElementById('savePayment');

    settingsBtn.addEventListener('click', () => {
        settingsModal.style.display = 'block';
        loadSettingsToForm();
    });
    saveSettingsBtn.addEventListener('click', () => {
        saveSettings();
        settingsModal.style.display = 'none';
    });

    addPaymentBtn.addEventListener('click', () => {
        addPaymentModal.style.display = 'block';
        document.getElementById('paymentDate').value = new Date().toISOString().split('T')[0];
    });
    savePaymentBtn.addEventListener('click', () => {
        addPaymentRecord();
        addPaymentModal.style.display = 'none';
    });

    document.querySelectorAll('.close').forEach(btn => {
        btn.addEventListener('click', e => {
            document.getElementById(e.target.dataset.modal).style.display = 'none';
        });
    });
    window.addEventListener('click', e => {
        if (e.target.classList.contains('modal')) e.target.style.display = 'none';
    });
}

// ==================== CÀI ĐẶT NGƯỠNG ====================
function loadSettingsToForm() {
    document.getElementById('room1PowerThreshold').value = settings.room1PowerThreshold;
    document.getElementById('room2PowerThreshold').value = settings.room2PowerThreshold;
    document.getElementById('room1CostThreshold').value = settings.room1CostThreshold;
    document.getElementById('room2CostThreshold').value = settings.room2CostThreshold;
    document.getElementById('electricityPrice').value = settings.electricityPrice;
}

function saveSettings() {
    const r1 = parseFloat(document.getElementById('room1PowerThreshold').value);
    const r2 = parseFloat(document.getElementById('room2PowerThreshold').value);
    const price = parseFloat(document.getElementById('electricityPrice').value);

    settings.electricityPrice = price;
    settings.room1PowerThreshold = r1;
    settings.room2PowerThreshold = r2;
    settings.room1CostThreshold = r1 * price;
    settings.room2CostThreshold = r2 * price;

    document.getElementById('room1CostThreshold').value = settings.room1CostThreshold;
    document.getElementById('room2CostThreshold').value = settings.room2CostThreshold;

    localStorage.setItem('roomMonitorSettings', JSON.stringify(settings));
    database.ref('C000001').update({ Threshold1: r1 });
    database.ref('C000002').update({ Threshold2: r2 });
    database.ref().update({UnitPrice: price});

    alert('✅ Đã lưu & đồng bộ Firebase!');
    updateAlerts();
}

function loadSettings() {
    const saved = localStorage.getItem('roomMonitorSettings');
    if (saved) settings = JSON.parse(saved);
}

// ==================== THANH TOÁN ====================
function loadPaymentData() {
    const saved = localStorage.getItem('paymentData');
    paymentData = saved ? JSON.parse(saved) : [];
}

function savePaymentData() {
    localStorage.setItem('paymentData', JSON.stringify(paymentData));
}

function addPaymentRecord() {
    const date = document.getElementById('paymentDate').value;
    const room = document.getElementById('paymentRoom').value;
    const amount = parseFloat(document.getElementById('paymentAmount').value);
    const status = document.getElementById('paymentStatus').value;

    if (!date || !amount) return alert('⚠️ Điền đầy đủ thông tin!');
    const record = { date, room, amount, status };
    paymentData.unshift(record);
    savePaymentData();
    renderPaymentTable();
    alert('✅ Đã thêm bản ghi!');
}

function deletePaymentRecord(index) {
    if (confirm('🗑️ Bạn có chắc muốn xóa bản ghi này không?')) {
        paymentData.splice(index, 1);
        savePaymentData();
        renderPaymentTable();
    }
}

function renderPaymentTable() {
    const tbody = document.getElementById('paymentTableBody');
    tbody.innerHTML = '';
    paymentData.forEach((r, i) => {
        const d = new Date(r.date).toLocaleDateString('vi-VN');
        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td>${d}</td>
            <td>Phòng ${r.room}</td>
            <td>${r.amount.toLocaleString('vi-VN')} VNĐ</td>
            <td>
                <select class="status-select" data-index="${i}">
                    <option value="Đã đóng" ${r.status === 'Đã đóng' ? 'selected' : ''}>Đã đóng</option>
                    <option value="Chưa đóng" ${r.status === 'Chưa đóng' ? 'selected' : ''}>Chưa đóng</option>
                </select>
            </td>
            <td><button class="delete-btn" data-index="${i}">🗑️ Xóa</button></td>`;
        tbody.appendChild(tr);
    });

    document.querySelectorAll('.status-select').forEach(sel => {
        sel.addEventListener('change', e => {
            const i = e.target.dataset.index;
            paymentData[i].status = e.target.value;
            savePaymentData();
        });
    });

    document.querySelectorAll('.delete-btn').forEach(btn => {
        btn.addEventListener('click', e => {
            const i = e.target.dataset.index;
            deletePaymentRecord(i);
        });
    });
}