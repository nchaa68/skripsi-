document.addEventListener("DOMContentLoaded", function() {
    const kapasitasObatInput = document.getElementById("kapasitasObat");
    const maxKapasitas = 5; // Maksimal kuantitas obat
  
    kapasitasObatInput.addEventListener("input", function(event) {
      // Hapus semua karakter yang bukan angka menggunakan regular expression
      event.target.value = event.target.value.replace(/[^0-9]/g, "");
  
      // Batasi nilai kuantitas obat menjadi maksimal
      if (event.target.value > maxKapasitas) {
        event.target.value = maxKapasitas;
      }
    });
  });

document.addEventListener("DOMContentLoaded", function() {
    const datePicker = flatpickr("#tanggal", {
      mode: "range", // Enable range mode
      dateFormat: "d-m-Y", // Date format
      minDate: "today", // Minimum selectable date is today
      maxDate: new Date().fp_incr(360),// Maximum selectable date is 7 days from today
      locale: {
        firstDayOfWeek: 1, // Set Monday as the first day of the week
      }
    });
  
    // Rest of your code
  });