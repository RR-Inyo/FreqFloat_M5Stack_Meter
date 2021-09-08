// FloatFreq data acquisition and record
// Data aquisition every 10 second
// (c) 2021 @RR_Inyo
// Released under the MIT lisence.
// https://opensource.org/licenses/mit-license.php

function doPost(e) {
  // Acquire FloatFreq data from parameter
  var params = JSON.parse(e.postData.getDataAsString());
  sensedDate = params.date;
  freq = params.freq;
   
  // Obtain spreadsheet handler
  var ss = SpreadsheetApp.getActiveSpreadsheet();

  // Obtain handler for sheets
  var sheetDashboard = ss.getSheetByName("dashboard");
  var sheetData = ss.getSheetByName("data");

  // Add data
  sheetData.appendRow([sensedDate, freq]);
  
  // Obtain the last row
  rowLast = sheetData.getLastRow();

  // Send email in case of abnormal frequency event
  var mailRecipients = "example@example.com"

  var alarmONOFF = sheetDashboard.getRange("alarm").getValue();

  var f_under = sheetDashboard.getRange("f_under").getValue();
  var f_over = sheetDashboard.getRange("f_over").getValue();
  var freq_old = sheetData.getRange(rowLast - 1, 2).getValue();

  if (alarmONOFF == "ON") {
    if (freq < f_under && freq_old > f_under) {
      GmailApp.sendEmail(mailRecipients, "Low Frequency Alarm","Mains frequency dropped to" + String(freq) + " Hz.");
    }

    if (freq > f_over && freq_old < f_over) {
      GmailApp.sendEmail(mailRecipients, "High Frequency Alarm","Mains frequency rose to" + String(freq) + " Hz.");
    }
  }
  
  // Calculte moving average
  sheetData.getRange(rowLast, 3).setFormulaR1C1("=average(R[-18]C[-1]:R[0]C[-1])");

  // Sort the last twelve rows in case older data come later
  if (rowLast > 12) {
    lastFiveRows = sheetData.getRange(rowLast - 11, 1, 12, 3);
    lastFiveRows.sort([{column: 1, ascending: true}]);
  }
  
  // Set new ranges for graphs
  oneHour = 6 * 60;
  newRangeShort = sheetData.getRange(Math.max(rowLast - oneHour + 1, 1), 1, oneHour, 3);

  oneDay = 6 * 60 * 24;
  newRangeLong = sheetData.getRange(Math.max(rowLast - oneDay + 1, 1), 1, oneDay, 3);

  // Obtain graph handler
  var graphs_Fs = sheetDashboard.getCharts()[0];
  var graphs_Fl = sheetDashboard.getCharts()[1];

  // Update graph ranges
  graphs_Fs = graphs_Fs.modify()
    .clearRanges()
    .asAreaChart()
    .setTitle("Frequency [Hz] (60 min)")
    .setColors(["blue", "orange"])
    .addRange(newRangeShort)
    .build();

  graphs_Fl = graphs_Fl.modify()
    .clearRanges()
    .asAreaChart()
    .setTitle("Frequency [Hz] (24 h)")
    .setColors(["blue", "orange"])
    .addRange(newRangeLong)
    .build();

  // Uodate graph
  sheetDashboard.updateChart(graphs_Fs);
  sheetDashboard.updateChart(graphs_Fl);

  // Delete old data if number of row exceeds 80000
  if (rowLast > 80000) {
    sheetData.deleteRow(2);
  }
}
