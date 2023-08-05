#include "Battery18650Stats.h"

Battery18650Stats::Battery18650Stats(int adcPin, double conversionFactor, int reads, double maxVoltage, double minVoltage) { 
  _reads = reads;
  _conversionFactor = conversionFactor;
  _adcPin = adcPin;
  _maxVoltage = maxVoltage;
  _minVoltage = minVoltage;
}

Battery18650Stats::Battery18650Stats(int adcPin, double conversionFactor, int reads) { 
  _reads = reads;
  _conversionFactor = conversionFactor;
  _adcPin = adcPin;
  _maxVoltage = DEFAULT_MAX_VOLTAGE;
  _minVoltage = DEFAULT_MIN_VOLTAGE;
}

Battery18650Stats::Battery18650Stats(int adcPin, double conversionFactor) {
  _reads = DEFAULT_READS;
  _conversionFactor = conversionFactor;
  _adcPin = adcPin;
  _maxVoltage = DEFAULT_MAX_VOLTAGE;
  _minVoltage = DEFAULT_MIN_VOLTAGE;
}

Battery18650Stats::Battery18650Stats(int adcPin) {
  _reads = DEFAULT_READS;
  _conversionFactor = DEFAULT_CONVERSION_FACTOR;
  _adcPin = adcPin;
  _maxVoltage = DEFAULT_MAX_VOLTAGE;
  _minVoltage = DEFAULT_MIN_VOLTAGE;
}

Battery18650Stats::Battery18650Stats() {
  _reads = DEFAULT_READS;
  _conversionFactor = DEFAULT_CONVERSION_FACTOR;
  _adcPin = DEFAULT_PIN;
  _maxVoltage = DEFAULT_MAX_VOLTAGE;
  _minVoltage = DEFAULT_MIN_VOLTAGE;
}

Battery18650Stats::~Battery18650Stats() {
  free(_conversionTable);
  delete this->_conversionTable;
}

double Battery18650Stats::getBatteryVolts() {
  int readValue = _avgAnalogRead(_adcPin, _reads);
  return _analogReadToVolts(readValue);
}

int Battery18650Stats::getBatteryChargeLevel(bool useConversionTable) {
  int readValue = _avgAnalogRead(_adcPin, _reads);
  double volts = _analogReadToVolts(readValue);

  if (volts >= _maxVoltage) {
    return 100;
  }

  if (volts <= _minVoltage) {
    return 0;
  }

  return useConversionTable
         ? _getChargeLevelFromConversionTable(volts)
         : _calculateChargeLevel(volts);
}

int Battery18650Stats::pinRead(){
    return _avgAnalogRead(_adcPin, _reads); 
}

void Battery18650Stats::_initConversionTable() {
  _conversionTable = (double*)malloc(sizeof(double)*101);
  double voltageStep = (_maxVoltage - _minVoltage) / 100;
  double currentVoltage = _minVoltage;
  
  for (int i = 0; i <= 100; i++) {
    _conversionTable[i] = currentVoltage;
    currentVoltage += voltageStep;
  }
}

int Battery18650Stats::_avgAnalogRead(int pinNumber, int reads) {
  int totalValue = 0;
  for (int i = 0; i < reads; i++) {
    totalValue += analogRead(pinNumber);
  }
  return (int) (totalValue / reads);
}

int Battery18650Stats::_calculateChargeLevel(double volts) {
  if (volts <= 3.700) {
    return (20 * volts) - 64;
  }

  int chargeLevel = round((-233.82 * volts * volts) + (2021.3 * volts) - 4266);
  return ((volts > 3.755 && volts <= 3.870) || volts >= 3.940)
    ? chargeLevel + 1
    : chargeLevel;
}

int Battery18650Stats::_getChargeLevelFromConversionTable(double volts) {
  if (_conversionTable == nullptr) {
    _initConversionTable();
  }

  int index = 50;
  int previousIndex = 0;
  int half = 0;

  while(previousIndex != index) {
    half = abs(index - previousIndex) / 2;
    previousIndex = index;
    if (_conversionTable[index] == volts) {
      return index;
    }
    index = (volts >= _conversionTable[index])
       ? index + half
       : index - half;
  }

  return index;
}

double Battery18650Stats::_analogReadToVolts(int readValue) {
  return readValue * _conversionFactor / 1000;
}


