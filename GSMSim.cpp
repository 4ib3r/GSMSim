/*
	GSMSim Library

	This library written for SIMCOM Sim800L module. Library may worked on any SIMCOM modules
	and GSM Shields.

	Created 11.05.2017
	By Erdem ARSLAN
	Modified 30.08.2017

	Erdem ARSLAN
	Science and Technology Teacher, an Arduino Lover =)
	erdemsaid@gmail.com
	https://www.erdemarslan.com/
	http://www.robothane.net/
	http://www.eralabs.net/

*/

#include "Arduino.h"
#include "GSMSim.h"
#include <SoftwareSerial.h>

GSMSim::GSMSim(void) : SoftwareSerial(DEFAULT_RX_PIN, DEFAULT_TX_PIN)
{
	RX_PIN = DEFAULT_RX_PIN;
	TX_PIN = DEFAULT_TX_PIN;
	RESET_PIN = DEFAULT_RST_PIN;
	LED_PIN = DEFAULT_LED_PIN;
	LED_FLAG = DEFAULT_LED_FLAG;
}

GSMSim::GSMSim(uint8_t rx, uint8_t tx) : SoftwareSerial(rx, tx)
{
	RX_PIN = rx;
	TX_PIN = tx;
	RESET_PIN = DEFAULT_RST_PIN;
	LED_PIN = DEFAULT_LED_PIN;
	LED_FLAG = DEFAULT_LED_FLAG;
}

GSMSim::GSMSim(uint8_t rx, uint8_t tx, uint8_t rst) : SoftwareSerial(rx, tx)
{
	RX_PIN = rx;
	TX_PIN = tx;
	RESET_PIN = rst;
	LED_PIN = DEFAULT_LED_PIN;
	LED_FLAG = DEFAULT_LED_FLAG;
}

GSMSim::GSMSim(uint8_t rx, uint8_t tx, uint8_t rst, uint8_t led) : SoftwareSerial(rx, tx)
{
	RX_PIN = rx;
	TX_PIN = tx;
	RESET_PIN = rst;
	LED_PIN = led;
	LED_FLAG = DEFAULT_LED_FLAG;
}

GSMSim::GSMSim(uint8_t rx, uint8_t tx, uint8_t rst, uint8_t led, bool ledflag) : SoftwareSerial(rx, tx)
{
	RX_PIN = rx;
	TX_PIN = tx;
	RESET_PIN = rst;
	LED_PIN = led;
	LED_FLAG = ledflag;
}

// Start GSMSim
void GSMSim::start()
{

	pinMode(RESET_PIN, OUTPUT);
	digitalWrite(RESET_PIN, HIGH);

	_baud = DEFAULT_BAUD_RATE;

	this->begin(_baud);

	if (LED_FLAG)
	{
		pinMode(LED_PIN, OUTPUT);
	}

	_buffer.reserve(BUFFER_RESERVE_MEMORY);
}
void GSMSim::start(uint32_t baud)
{
	pinMode(RESET_PIN, OUTPUT);
	digitalWrite(RESET_PIN, HIGH);

	_baud = baud;

	this->begin(_baud);

	if (LED_FLAG)
	{
		pinMode(LED_PIN, OUTPUT);
	}

	_buffer.reserve(BUFFER_RESERVE_MEMORY);
}

String GSMSim::readSerial()
{
	return _readSerial();
}

String GSMSim::readSerial(uint32_t timeout)
{
	return _readSerial(timeout);
}

// Reset GMS Module
void GSMSim::reset()
{
	if (LED_FLAG)
	{
		digitalWrite(LED_PIN, HIGH);
	}

	digitalWrite(RESET_PIN, LOW);
	delay(1000);
	digitalWrite(RESET_PIN, HIGH);
	delay(1000);

	// Modul kendine geldi mi onu bekle
	this->print(F("AT\r"));
	while (_readSerial().indexOf("OK") == -1)
	{
		this->print(F("AT\r"));
	}

	if (LED_FLAG)
	{
		digitalWrite(LED_PIN, LOW);
	}
}

// SET PHONE FUNC
bool GSMSim::setPhoneFunc(uint8_t level = 1)
{
	if (level != 0 || level != 1 || level != 4)
	{
		return false;
	}
	else
	{
		this->print(F("AT+CFUN="));
		this->print(String(level));
		this->print(F("\r"));

		_buffer = _readSerial();
		if ((_buffer.indexOf("OK")) != -1)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

// SIGNAL QUALTY - 0-31 | 0-> poor | 31 - Full | 99 -> Unknown
uint8_t GSMSim::signalQuality()
{
	this->print(F("AT+CSQ\r"));
	_buffer = _readSerial();

	if ((_buffer.indexOf("+CSQ:")) != -1)
	{
		return _buffer.substring(_buffer.indexOf("+CSQ: ") + 6, _buffer.indexOf(",")).toInt();
	}
	else
	{
		return 99;
	}
}

// IS Module connected to the operator?
bool GSMSim::isRegistered()
{
	this->print(F("AT+CREG?\r"));
	_buffer = _readSerial();

	if ((_buffer.indexOf("+CREG: 0,1")) != -1 || (_buffer.indexOf("+CREG: 0,5")) != -1 || (_buffer.indexOf("+CREG: 1,1")) != -1 || (_buffer.indexOf("+CREG: 1,5")) != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// IS SIM Inserted?
bool GSMSim::isSimInserted()
{
	this->print(F("AT+CSMINS?\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf(",") != -1)
	{
		// bölelim
		String data = _buffer.substring(_buffer.indexOf(","), _buffer.indexOf("OK"));
		data.trim();
		if (data == "1")
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

// Pin statüsü - AT+CPIN?
uint8_t GSMSim::pinStatus()
{
	this->print(F("AT+CPIN?\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("READY") != -1)
	{
		return 0;
	}
	else if (_buffer.indexOf("SIM PIN") != -1)
	{
		return 1;
	}
	else if (_buffer.indexOf("SIM PUK") != -1)
	{
		return 2;
	}
	else if (_buffer.indexOf("PH_SIM PIN") != -1)
	{
		return 3;
	}
	else if (_buffer.indexOf("PH_SIM PUK") != -1)
	{
		return 4;
	}
	else if (_buffer.indexOf("SIM PIN2") != -1)
	{
		return 5;
	}
	else if (_buffer.indexOf("SIM PUK2") != -1)
	{
		return 6;
	}
	else
	{
		return 7;
	}
}

// OPERATOR NAME
String GSMSim::operatorName()
{
	this->print(F("AT+COPS?\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf(",") == -1)
	{
		return "NOT CONNECTED";
	}
	else
	{
		return _buffer.substring(_buffer.indexOf(",\"") + 2, _buffer.lastIndexOf("\""));
	}
}

// OPERATOR NAME FROM SIM
String GSMSim::operatorNameFromSim()
{
	this->flush();
	this->print(F("AT+CSPN?\r"));
	_buffer = _readSerial();
	delay(250);
	_buffer = _readSerial();
	/*
	return _buffer;
	*/
	if (_buffer.indexOf("OK") != -1)
	{
		return _buffer.substring(_buffer.indexOf(" \"") + 2, _buffer.lastIndexOf("\""));
	}
	else
	{
		return "NOT CONNECTED";
	}
}

// PHONE STATUS
uint8_t GSMSim::phoneStatus()
{
	this->print(F("AT+CPAS\r"));
	_buffer = _readSerial();

	if ((_buffer.indexOf("+CPAS: ")) != -1)
	{
		return _buffer.substring(_buffer.indexOf("+CPAS: ") + 7, _buffer.indexOf("+CPAS: ") + 9).toInt();
	}
	else
	{
		return 99; // not read from module
	}
}

// ECHO OFF
bool GSMSim::echoOff()
{
	this->print(F("ATE0\r"));
	_buffer = _readSerial();
	if ((_buffer.indexOf("OK")) != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// ECHO ON
bool GSMSim::echoOn()
{
	this->print(F("ATE1\r"));
	_buffer = _readSerial();
	if ((_buffer.indexOf("OK")) != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Modül Üreticisi
String GSMSim::moduleManufacturer()
{
	this->print(F("AT+CGMI\r"));
	_buffer = _readSerial();
	String data = _buffer.substring(8, _buffer.indexOf("OK"));
	data.trim();
	data.replace("_", " ");
	return data;
}

// Modül Modeli
String GSMSim::moduleModel()
{
	this->print(F("AT+CGMM\r"));
	_buffer = _readSerial();

	String data = _buffer.substring(8, _buffer.indexOf("OK"));
	data.trim();
	data.replace("_", " ");
	return data;
}

// Modül Revizyonu
String GSMSim::moduleRevision()
{
	this->print(F("AT+CGMR\r"));
	_buffer = _readSerial();

	String data = _buffer.substring(_buffer.indexOf(":") + 1, _buffer.indexOf("OK"));
	data.trim();
	return data;
}

// Modülün IMEI numarası
String GSMSim::moduleIMEI()
{
	this->print(F("AT+CGSN\r"));
	_buffer = _readSerial();

	String data = _buffer.substring(8, _buffer.indexOf("OK"));
	data.trim();
	return data;
}

// Modülün IMEI Numarasını değiştirir.
bool GSMSim::moduleIMEIChange(char *imeino)
{
	return true;
}

// Modülün SIM Numarası
String GSMSim::moduleIMSI()
{
	this->print(F("AT+CIMI\r"));
	_buffer = _readSerial();

	String data = _buffer.substring(8, _buffer.indexOf("OK"));
	data.trim();
	return data;
}

// Sim Kart Seri Numarası
String GSMSim::moduleICCID()
{
	this->print(F("AT+CCID\r"));
	_buffer = _readSerial();

	String data = _buffer.substring(8, _buffer.indexOf("OK"));
	data.trim();

	return data;
}

// Çalma Sesi
uint8_t GSMSim::ringerVolume()
{
	this->print(F("AT+CRSL?\r"));
	_buffer = _readSerial();

	String data = _buffer.substring(7, _buffer.indexOf("OK"));
	data.trim();

	return data.toInt();
}

// Çalma sesini ayarla
bool GSMSim::setRingerVolume(uint8_t level)
{
	if (level > 100)
	{
		level = 100;
	}

	this->print(F("AT+CRSL="));
	this->print(level);
	this->print(F("\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Hoparlör sesi
uint8_t GSMSim::speakerVolume()
{
	this->print(F("AT+CLVL?\r"));
	_buffer = _readSerial();

	String data = _buffer.substring(7, _buffer.indexOf("OK"));
	data.trim();

	return data.toInt();
}

// Hoparlör sesini ayarla
bool GSMSim::setSpeakerVolume(uint8_t level)
{
	if (level > 100)
	{
		level = 100;
	}

	this->print(F("AT+CLVL="));
	this->print(level);
	this->print(F("\r"));

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

String GSMSim::moduleDebug()
{
	this->print(F("AT&V\r"));

	return _readSerial();
}

//////////////////////////////////////
//			CALL	SECTION			//
//////////////////////////////////////

// Arama Yapar
bool GSMSim::call(char *phone_number)
{

	bool sorgulamaYapma = callIsCOLPActive();
	_buffer = _readSerial();
	delay(100);

	this->print(F("ATD"));
	this->print(phone_number);
	this->print(";\r");

	if (sorgulamaYapma)
	{
		return true;
	}
	else
	{
		_buffer = _readSerial();

		if (_buffer.indexOf("OK") != -1)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

// Gelen aramayı cevaplar
bool GSMSim::callAnswer()
{
	this->print(F("ATA\r"));

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Aramayı reddeder veya görüşmeyi sonlandırır!
bool GSMSim::callHangoff()
{
	this->print(F("ATH\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Arama durumunu belirtir
uint8_t GSMSim::callStatus()
{
	/*
		values of return:
		0 Ready (MT allows commands from TA/TE)
		2 Unknown (MT is not guaranteed to respond to tructions)
		3 Ringing (MT is ready for commands from TA/TE, but the ringer is active)
		4 Call in progress
	*/
	this->print(F("AT+CPAS\r"));
	_buffer = _readSerial();
	return _buffer.substring(_buffer.indexOf("+CPAS: ") + 7, _buffer.indexOf("+CPAS: ") + 9).toInt();
}

// Connected Line Identification aktif veya kapalı
bool GSMSim::callSetCOLP(bool active)
{
	int status = active == true ? 1 : 0;
	this->print(F("AT+COLP="));
	this->print(status);
	this->print("\r");

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// COLP Aktif mi değil mi?
bool GSMSim::callIsCOLPActive()
{
	this->print("AT+COLP?\r");
	_buffer = _readSerial();

	if (_buffer.indexOf("+COLP: 1") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Arayanı söyleme aktif mi değil mi?
bool GSMSim::callActivateListCurrent(bool active)
{
	int status = active == true ? 1 : 0;
	this->print(F("AT+CLCC="));
	this->print(status);
	this->print("\r");

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}
// şimdi arayanı söyle
String GSMSim::callReadCurrentCall(String serialRaw)
{

	String result = "";
	if (serialRaw.indexOf("+CLCC:") != -1)
	{
		String status = serialRaw.substring(11, 13);
		String number = serialRaw.substring(18, serialRaw.indexOf("\","));

		if (status == "0")
		{
			status = "STATUS:ACTIVE"; // Görüşme var
		}
		else if (status == "1")
		{
			status = "STATUS:HELD";
		}
		else if (status == "2")
		{
			status = "STATUS:DIALING"; // Çevriliyor
		}
		else if (status == "3")
		{
			status = "STATUS:ALERTING"; // Çalıyor
		}
		else if (status == "4")
		{
			status = "STATUS:INCOMING"; // Gelen arama
		}
		else if (status == "5")
		{
			status = "STATUS:WAITING"; // gelen arama bekliyor
		}
		else if (status == "6")
		{
			status = "STATUS:DISCONNECT"; // görüşme bitti
		}

		result = status + "|NUMBER:" + number;
	}

	return result;
}

//////////////////////////////////////
//			msg BÖLÜMÜ			//
//////////////////////////////////////

// SMS i TEXT ya da PDU moduna alır.
bool GSMSim::smsTextMode(bool textModeON)
{
	if (textModeON == true)
	{
		this->print(F("AT+CMGF=1\r"));
	}
	else
	{
		this->print(F("AT+CMGF=0\r"));
	}
	bool result = false;
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") != -1)
	{
		result = true;
	}

	return result;
}

// datalen number ve mesajı gönderir!
bool GSMSim::smsSend(char *number, char *message)
{
	this->print(F("AT+CMGS=\"")); // command to send sms
	this->print(number);
	this->print(F("\"\r"));
	_buffer = _readSerial();
	this->print(message);
	this->print("\r");
	//change delay 100 to readserial
	_buffer += _readSerial();
	this->print((char)26);

	_buffer += _readSerial();
	//expect CMGS:xxx   , where xxx is a number,for the sending sms.
	/*
	return _buffer;
	*/
	if (((_buffer.indexOf("AT+CMGS")) != -1))
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Belirtilen klasördeki smslerin indexlerini listeler!
String GSMSim::smsListUnread()
{

	this->print(F("AT+CMGL=\"REC UNREAD\",1\r"));

	_buffer = _readSerial();

	String donus = "";

	if (_buffer.indexOf("ERROR") != -1)
	{
		donus = "ERROR";
	}

	if (_buffer.indexOf("+CMGL:") != -1)
	{

		String data = _buffer;
		bool islem = false;
		donus = "";

		while (!islem)
		{
			if (data.indexOf("+CMGL:") == -1)
			{
				islem = true;
				continue;
			}

			data = data.substring(data.indexOf("+CMGL: ") + 7);
			String metin = data.substring(0, data.indexOf(","));
			metin.trim();

			if (donus == "")
			{
				donus += "SMSIndexNo:";
				donus += metin;
			}
			else
			{
				donus += ",";
				donus += metin;
			}
		}
	}
	else
	{
		if (donus != "ERROR")
		{
			donus = "NO_SMS";
		}
	}

	return donus;
}

// Indexi datalen mesajı okur. Anlaşılır hale getirir!
String GSMSim::smsRead(uint8_t index)
{
	this->print("AT+CMGR=");
	this->print(index);
	this->print(",0\r");

	_buffer = _readSerial();

	String status = "INDEX_NO_ERROR";

	if (_buffer.indexOf("+CMGR:") != -1)
	{

		String cls, status, telno, time, msg;

		cls = "UNKNOWN";
		status = "UNKNOWN";

		if (_buffer.indexOf("REC UNREAD") != -1)
		{
			cls = "INCOMING";
			status = "UNREAD";
		}
		if (_buffer.indexOf("REC READ") != -1)
		{
			cls = "INCOMING";
			status = "READ";
		}
		if (_buffer.indexOf("STO UNSENT") != -1)
		{
			cls = "OUTGOING";
			status = "UNSENT";
		}
		if (_buffer.indexOf("STO SENT") != -1)
		{
			cls = "OUTGOING";
			status = "SENT";
		}

		String callNo = _buffer.substring(_buffer.indexOf("\",\"") + 3);
		telno = callNo.substring(0, callNo.indexOf("\",\"")); // telefon numarası tamam

		String history = callNo.substring(callNo.lastIndexOf("\",\"") + 3);

		time = history.substring(0, history.indexOf("\"")); // timeı da aldık. Bir tek msg kaldı!

		msg = history.substring(history.indexOf("\"") + 1, history.lastIndexOf("OK"));

		msg.trim();

		status = "FOLDER:";
		status += cls;
		status += "|STATUS:";
		status += status;
		status += "|PHONENO:";
		status += telno;
		status += "|DATETIME:";
		status += time;
		status += "|MESSAGE:";
		status += msg;
	}

	return status;
}
// Indexi datalen mesajı okur. Anlaşılır hale getirir!
String GSMSim::smsRead(uint8_t index, bool markRead)
{
	this->print("AT+CMGR=");
	this->print(index);
	this->print(",");
	if (markRead == true)
	{
		this->print("0");
	}
	else
	{
		this->print("1");
	}
	this->print("\r");

	_buffer = _readSerial();

	String status = "INDEX_NO_ERROR";

	if (_buffer.indexOf("+CMGR:") != -1)
	{

		String cls, status, telno, time, msg;

		cls = "UNKNOWN";
		status = "UNKNOWN";

		if (_buffer.indexOf("REC UNREAD") != -1)
		{
			cls = "INCOMING";
			status = "UNREAD";
		}
		if (_buffer.indexOf("REC READ") != -1)
		{
			cls = "INCOMING";
			status = "READ";
		}
		if (_buffer.indexOf("STO UNSENT") != -1)
		{
			cls = "OUTGOING";
			status = "UNSENT";
		}
		if (_buffer.indexOf("STO SENT") != -1)
		{
			cls = "OUTGOING";
			status = "SENT";
		}

		String callNo = _buffer.substring(_buffer.indexOf("\",\"") + 3);
		telno = callNo.substring(0, callNo.indexOf("\",\"")); // telefon numarası tamam

		String history = callNo.substring(callNo.lastIndexOf("\",\"") + 3);

		time = history.substring(0, history.indexOf("\"")); // timeı da aldık. Bir tek msg kaldı!

		msg = history.substring(history.indexOf("\"") + 1, history.lastIndexOf("OK"));

		msg.trim();

		status = "FOLDER:";
		status += cls;
		status += "|STATUS:";
		status += status;
		status += "|PHONENO:";
		status += telno;
		status += "|DATETIME:";
		status += time;
		status += "|MESSAGE:";
		status += msg;
	}

	return status;
}

// Serialden Mesajı okur
String GSMSim::smsReadFromSerial(String serialRaw)
{
	if (serialRaw.indexOf("+CMTI:") != -1)
	{
		String number = serialRaw.substring(serialRaw.indexOf("\",") + 2);
		int no = number.toInt();

		return smsRead(no, true);
	}
	else
	{
		return "RAW_DATA_NOT_READ";
	}
}

// serialden mesajın indexini alır
uint8_t GSMSim::smsIndexFromSerial(String serialRaw)
{
	if (serialRaw.indexOf("+CMTI:") != -1)
	{
		String number = serialRaw.substring(serialRaw.indexOf("\",") + 2);
		int no = number.toInt();

		return no;
	}
	else
	{
		return -1;
	}
}

// msg merkez numasını getirir
String GSMSim::smsReadMessageCenter()
{
	this->print("AT+CSCA?\r");
	_buffer = _readSerial();

	String result = "";

	if (_buffer.indexOf("+CSCA:") != -1)
	{
		result = _buffer.substring(_buffer.indexOf("+CSCA:") + 8, _buffer.indexOf("\","));
	}

	return result;
}

// msg merkez numarasını değiştirir
bool GSMSim::smsChangeMessageCenter(char *messageCenter)
{
	this->print("AT+CSCA=\"");
	this->print(messageCenter);
	this->print("\"\r");

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// tek bir mesajı siler
bool GSMSim::smsDeleteOne(uint8_t index)
{
	this->print(F("AT+CMGD="));
	this->print(index);
	this->print(F(",0\r"));

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Tüm okunmuş mesajlaarı siler. Fakat gidene dokunmaz
bool GSMSim::smsDeleteAllRead()
{
	this->print(F("AT+CMGD=1,1\r"));

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// okunmuş okunmamış ne varsa siler
bool GSMSim::smsDeleteAll()
{
	this->print(F("AT+CMGD=1,4\r"));

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Gelen msg Indicatorünü değiştir
bool GSMSim::smsChangeIncomingIndicator(bool save)
{
	if (save)
	{
		this->print(F("AT+CNMI=1,1,0,0,0\r"));
	}
	else
	{
		this->print(F("AT+CNMI=2,2,0,0,0\r"));
	}

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//////////////////////////////////////
//			DTMF BÖLÜMÜ				//
//////////////////////////////////////

// DTMF yi ayarlar
bool GSMSim::dtmfSet(bool active, uint8_t interval, bool reportTime, bool soundDetect)
{
	int mode = active == true ? 1 : 0;
	int rtime = reportTime == true ? 1 : 0;
	int ssdet = soundDetect == true ? 1 : 0;

	this->print(F("AT+DDET="));
	this->print(mode);
	this->print(F(","));
	this->print(interval);
	this->print(F(","));
	this->print(rtime);
	this->print(F(","));
	this->print(ssdet);
	this->print(F("\r"));

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}
// Serialden DTMF Yi okur ve karakter olarak geri döner!
String GSMSim::dtmfRead(String serialRaw)
{

	if (serialRaw.indexOf("+DTMF:") != -1)
	{
		//  var mı yok mu?
		String metin;
		if (serialRaw.indexOf(",") != -1)
		{
			metin = serialRaw.substring(7, serialRaw.indexOf(","));
		}
		else
		{
			metin = serialRaw.substring(7);
		}

		return metin;
	}
	else
	{
		return "?";
	}
}

//////////////////////////////////////
//			USSD SECTION			//
//////////////////////////////////////
// USSD kodu gönderir
bool GSMSim::ussdSend(char *code)
{
	this->print(F("AT+CUSD=1,\""));
	this->print(code);
	this->print(F("\"\r"));

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}
// Raw datadan cevabı okur!
String GSMSim::ussdRead(String serialRaw)
{
	if (serialRaw.indexOf("+CUSD:") != -1)
	{
		String metin = serialRaw.substring(serialRaw.indexOf(",\"") + 2, serialRaw.indexOf("\","));
		return metin;
	}
	else
	{
		return "NOT_USSD_RAW";
	}
}

//////////////////////////////////////
//			FM RADIO SECTION		//
//////////////////////////////////////

// SIM800L & SIM800H only

// FM RADIO Open
bool GSMSim::fmOpen()
{
	this->print(F("AT+FMOPEN=0\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool GSMSim::fmOpen(bool mainChannel)
{
	uint8_t channel = mainChannel == true ? 1 : 0;
	this->print(F("AT+FMOPEN="));
	this->print(channel);
	this->print(F("\r"));

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool GSMSim::fmOpen(bool mainChannel, uint8_t freq)
{
	uint8_t channel = mainChannel == true ? 1 : 0;
	uint8_t frequency = (uint8_t)875;
	if (freq < 875)
	{
		frequency = (uint8_t)875;
	}
	if (freq > 1080)
	{
		frequency = (uint8_t)1080;
	}

	this->print(F("AT+FMOPEN="));
	this->print(channel);
	this->print(",");
	this->print(frequency);
	this->print(F("\r"));

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// FM IS OPENED?
bool GSMSim::fmIsOpened()
{
	this->print(F("AT+FMOPEN?\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("+FMOPEN: 1") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// FM RADIO CLOSE
bool GSMSim::fmClose()
{
	this->print(F("AT+FMCLOSE\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// GET FM RADIO FREQ
uint8_t GSMSim::fmGetFreq()
{
	this->print(F("AT+FMFREQ?\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("+FMFREQ:") != -1)
	{
		String result = _buffer.substring(_buffer.indexOf("+FMFREQ:") + 8);
		result.trim();
		return result.toInt();
	}
	else
	{
		return 0;
	}
}

// SET FM RADIO FREQ
bool GSMSim::fmSetFreq(uint8_t freq)
{
	this->print(F("AT+FMFREQ="));
	uint8_t frequency = (uint8_t)875;
	if (freq < 875)
	{
		frequency = (uint8_t)875;
	}
	if (freq > 1080)
	{
		frequency = (uint8_t)1080;
	}
	this->print(frequency);
	this->print(F("\r"));

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// GET FM RADIO FREQ
uint8_t GSMSim::fmGetVolume()
{
	this->print(F("AT+FMVOLUME?\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("+FMVOLUME:") != -1)
	{
		String result = _buffer.substring(_buffer.indexOf("+FMVOLUME:") + 10);
		result.trim();
		return result.toInt();
	}
	else
	{
		return 0;
	}
}

// SET FM RADIO FREQ
bool GSMSim::fmSetVolume(uint8_t volume)
{
	this->print(F("AT+FMVOLUME="));
	uint8_t vol = 0;
	if (volume < 0)
	{
		vol = 0;
	}
	if (volume > 6)
	{
		vol = 6;
	}
	this->print(vol);
	this->print(F("\r"));

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//////////////////////////////////////
//			GPRS METHODS			//
//////////////////////////////////////
// Connect to GPRS Bearer

bool GSMSim::gprsConnectBearer()
{
	this->print(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return false;

	delay(100);
	this->print(F("AT+SAPBR=3,1,\"APN\",\"internet\"\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return false;

	delay(100);

	this->print(F("AT+SAPBR=3,1,\"USER\",\"\"\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return false;

	delay(100);

	this->print(F("AT+SAPBR=3,1,\"PWD\",\"\"\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return false;

	delay(100);

	this->print("AT+SAPBR=1,1\r");
	_buffer = _readSerial();
	delay(50);
	_buffer += _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return false;

	this->print("AT+SAPBR=2,1\r");
	_buffer = _readSerial();

	if (_buffer.indexOf("\"0.0.0.0\"") != -1 || _buffer.indexOf("ERR") != -1)
		return false;

	return true;
}

bool GSMSim::gprsConnectBearer(String apn)
{
	this->print(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return false;

	delay(100);
	this->print(F("AT+SAPBR=3,1,\"APN\",\""));
	this->print(apn);
	this->print(F("\"\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return false;

	delay(100);

	this->print(F("AT+SAPBR=3,1,\"USER\",\"\"\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return false;

	delay(100);

	this->print(F("AT+SAPBR=3,1,\"PWD\",\"\"\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return false;

	delay(100);

	this->print("AT+SAPBR=1,1\r");
	_buffer = _readSerial();
	delay(50);
	_buffer += _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return false;

	this->print("AT+SAPBR=2,1\r");
	_buffer = _readSerial();

	if (_buffer.indexOf("\"0.0.0.0\"") != -1 || _buffer.indexOf("ERR") != -1)
		return false;

	return true;
}

bool GSMSim::gprsConnectBearer(String apn, String user, String password)
{
	this->print(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return false;

	delay(100);
	this->print(F("AT+SAPBR=3,1,\"APN\",\""));
	this->print(apn);
	this->print(F("\"\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return false;

	delay(100);

	this->print(F("AT+SAPBR=3,1,\"USER\",\""));
	this->print(user);
	this->print(F("\"\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return false;

	delay(100);

	this->print(F("AT+SAPBR=3,1,\"PWD\",\""));
	this->print(password);
	this->print(F("\"\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return false;

	delay(100);
	this->print("AT+SAPBR=1,1\r");
	_buffer = _readSerial();
	delay(50);
	_buffer += _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return false;

	this->print("AT+SAPBR=2,1\r");
	_buffer = _readSerial();

	if (_buffer.indexOf("\"0.0.0.0\"") != -1 || _buffer.indexOf("ERR") != -1)
		return false;

	return true;
}

// Check is GPRS connected?
bool GSMSim::gprsIsConnected()
{
	this->print(F("AT+SAPBR=2,1\r"));
	_buffer = _readSerial();
	delay(50);
	_buffer += _readSerial();

	if (_buffer.indexOf("ERR") != -1 || _buffer.indexOf("\"0.0.0.0\"") != -1)
		return false;

	return true;
}
// GET IP Address
String GSMSim::gprsGetIP()
{
	this->print(F("AT+SAPBR=2,1\r\n"));
	_buffer = _readSerial();
	delay(50);
	_buffer += _readSerial();

	if (_buffer.indexOf("ERR") != -1 || _buffer.indexOf("\"0.0.0.0\"") != -1)
		return "ERROR:NO_IP";

	if (_buffer.indexOf("+SAPBR:") != -1)
	{
		String data = _buffer.substring(_buffer.indexOf(",\"") + 2, _buffer.lastIndexOf("\""));
		data.trim();
		return data;
	}

	return "ERROR:NO_IP_FETCH";
}

bool GSMSim::gprsCloseConn()
{
	this->print(F("AT+SAPBR=0,1\r"));
	_buffer = _readSerial();
	delay(50);
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
		return true;

	return false;
}

String GSMSim::gprsHTTPGet(String url)
{
	bool https = false;
	if (url.startsWith("https://"))
	{
		// Remove https if SSL
		//url = url.substring(8); // Unsure if this has any effect.
		https = true;
	}
	if (!gprsIsConnected())
		return "GPRS_NOT_CONNECTED";

	// Terminate http connection, if it opened before!
	this->print(F("AT+HTTPTERM\r"));
	_buffer = _readSerial();
	this->print(F("AT+HTTPINIT\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_INIT_ERROR";

	if (https)
	{
		this->print("AT+HTTPSSL=1\r");
		_buffer = _readSerial();
		if (_buffer.indexOf("OK") == -1)
			return "HTTPSSL_ERROR";
	}
	this->print(F("AT+HTTPPARA=\"CID\",1\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_PARAMETER_ERROR";

	this->print(F("AT+HTTPPARA=\"URL\",\""));
	this->print(url);
	this->print("\"\r");
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_PARAMETER_ERROR";

	this->print(F("AT+HTTPACTION=0\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_ACTION_ERROR";

	delay(100);
	_buffer = _readSerial(10000);
	if (_buffer.indexOf("+HTTPACTION: 0,") == -1)
		return "HTTP_ACTION_READ_ERROR";

	String code = _buffer.substring(_buffer.indexOf(",") + 1, _buffer.lastIndexOf(","));
	String length = _buffer.substring(_buffer.lastIndexOf(",") + 1);

	String result = "METHOD:GET|HTTPCODE:";
	result += code;
	result += "|LENGTH:";
	result += length;

	// Bağlantıyı kapat!
	this->print(F("AT+HTTPTERM\r"));
	_buffer = _readSerial();

	result.trim();

	return result;
}

String GSMSim::gprsHTTPGet(String url, bool read)
{
	bool https = false;
	if (url.startsWith("https://"))
	{
		// Remove https if SSL
		//url = url.substring(8); // Unsure if this has any effect.
		https = true;
	}
	if (!gprsIsConnected())
		return "ERROR:GPRS_NOT_CONNECTED";

	// Terminate http connection, if it opened before!
	this->print(F("AT+HTTPTERM\r"));
	_buffer = _readSerial();
	this->print(F("AT+HTTPINIT\r\n"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_INIT_ERROR";

	if (https)
	{
		this->print("AT+HTTPSSL=1\r");
		_buffer = _readSerial();
		if (_buffer.indexOf("OK") == -1)
			return "HTTP_SSL_ERROR";
	}
	this->print(F("AT+HTTPPARA=\"CID\",1\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_PARAMETER_ERROR";

	this->print(F("AT+HTTPPARA=\"URL\",\""));
	this->print(url);
	this->print(F("\"\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return "HTTP_PARAMETER_ERROR";

	this->print(F("AT+HTTPACTION=0\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_ACTION_ERROR";

	delay(100);
	_buffer = _readSerial(10000);
	if (_buffer.indexOf("+HTTPACTION: 0,") == -1)
		return "HTTP_ACTION_READ_ERROR";

	String code = _buffer.substring(_buffer.indexOf(",") + 1, _buffer.lastIndexOf(","));
	String length = _buffer.substring(_buffer.lastIndexOf(",") + 1);
	code.trim();
	length.trim();

	this->print(F("AT+HTTPREAD\r"));
	_buffer = _readSerial(10000);

	String reading = "";

	if (_buffer.indexOf("+HTTPREAD:") == -1)
		return "ERROR:HTTP_READ_ERROR";

	String kriter = "+HTTPREAD: " + length;
	String data = _buffer.substring(_buffer.indexOf(kriter) + kriter.length(), _buffer.lastIndexOf("OK"));
	reading = data;

	String result = "METHOD:GET|HTTPCODE:";
	result += code;
	result += "|LENGTH:";
	result += length;
	result += "|DATA:";
	reading.trim();
	result += reading;

	this->print(F("AT+HTTPTERM\r"));
	_buffer = _readSerial();

	result.trim();

	return result;
}

String GSMSim::gprsHTTPPost(String url, String data)
{
	bool https = false;
	if (url.startsWith("https://"))
	{
		// Remove https if SSL
		//url = url.substring(8); // Unsure if this has any effect.
		https = true;
	}

	if (!gprsIsConnected())
		return "GPRS_NOT_CONNECTED";

	// Terminate http connection, if it opened before!
	this->print(F("AT+HTTPTERM\r"));
	_buffer = _readSerial();
	this->print(F("AT+HTTPINIT\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_INIT_ERROR";

	if (https)
	{
		// Set SSL if https
		this->print("AT+HTTPSSL=1\r");
		_buffer = _readSerial();
		if (_buffer.indexOf("OK") == -1)
			return "HTTPSSL_ERROR";
	}

	// Set bearer profile id
	this->print(F("AT+HTTPPARA=\"CID\",1\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_PARAMETER_ERROR";

	// Set url
	this->print(F("AT+HTTPPARA=\"URL\",\""));
	this->print(url);
	this->print("\"\r");
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_PARAMETER_ERROR";

	// Indicate that data will be transfered within 30 secods.
	this->print(F("AT+HTTPDATA="));
	this->print(data.length());
	this->print(F(",30000\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("DOWNLOAD") == -1)
		return "HTTP_DATA_ERROR";

	// Send data.
	this->print(data);
	this->print(F("\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_DOWNLOAD_DATA_ERROR";

	// Set action and perform request 1=POST
	this->print(F("AT+HTTPACTION=1\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_ACTION_ERROR";

	// Get the response.
	delay(100);
	_buffer = _readSerial(10000);
	if (_buffer.indexOf("+HTTPACTION: 1,") == -1)
		return "HTTP_ACTION_READ_ERROR";

	String code = _buffer.substring(_buffer.indexOf(",") + 1, _buffer.lastIndexOf(","));
	String length = _buffer.substring(_buffer.lastIndexOf(",") + 1);

	String result = "METHOD:POST|HTTPCODE:";
	result += code;
	result += "|LENGTH:";
	result += length;

	// Bağlantıyı kapat!
	this->print(F("AT+HTTPTERM\r"));
	_buffer = _readSerial();

	result.trim();

	return result;
}

String GSMSim::gprsHTTPPost(String url, String data, bool read)
{
	bool https = false;
	if (url.startsWith("https://"))
	{
		// Remove https if SSL
		//url = url.substring(8); // Unsure if this has any effect.
		https = true;
	}

	if (!gprsIsConnected())
		return "GPRS_NOT_CONNECTED";

	// Terminate http connection, if it opened before!
	this->print(F("AT+HTTPTERM\r"));
	_buffer = _readSerial();
	this->print(F("AT+HTTPINIT\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_INIT_ERROR";

	if (https)
	{
		// Set SSL if https
		this->print("AT+HTTPSSL=1\r");
		_buffer = _readSerial();
		if (_buffer.indexOf("OK") == -1)
			return "HTTPSSL_ERROR";
	}

	// Set bearer profile id
	this->print(F("AT+HTTPPARA=\"CID\",1\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_PARAMETER_ERROR";

	// Set url
	this->print(F("AT+HTTPPARA=\"URL\",\""));
	this->print(url);
	this->print("\"\r");
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_PARAMETER_ERROR";

	// Indicate that data will be transfered within 30 secods.
	this->print(F("AT+HTTPDATA="));
	this->print(data.length());
	this->print(F(",30000\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("DOWNLOAD") == -1)
		return "HTTP_DATA_ERROR";

	// Send data.
	this->print(data);
	this->print(F("\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_DOWNLOAD_DATA_ERROR";

	// Set action and perform request 1=POST
	this->print(F("AT+HTTPACTION=1\r"));
	_buffer = _readSerial();
	if (_buffer.indexOf("OK") == -1)
		return "HTTP_ACTION_ERROR";

	// Get the response.
	delay(100);
	_buffer = _readSerial(10000);
	if (_buffer.indexOf("+HTTPACTION: 1,") == -1)
		return "HTTP_ACTION_READ_ERROR";

	String code = _buffer.substring(_buffer.indexOf(",") + 1, _buffer.lastIndexOf(","));
	String length = _buffer.substring(_buffer.lastIndexOf(",") + 1);
	code.trim();
	length.trim();

	this->print(F("AT+HTTPREAD\r"));
	_buffer = _readSerial(10000);

	String reading = "";

	if (_buffer.indexOf("+HTTPREAD:") == -1)
		return "ERROR:HTTP_READ_ERROR";

	String kriter = "+HTTPREAD: " + length;
	String dataString = _buffer.substring(_buffer.indexOf(kriter) + kriter.length(), _buffer.lastIndexOf("OK"));
	reading = dataString;

	String result = "METHOD:POST|HTTPCODE:";
	result += code;
	result += "|LENGTH:";
	result += length;
	result += "|DATA:";
	reading.trim();
	result += reading;

	this->print(F("AT+HTTPTERM\r"));
	_buffer = _readSerial();

	result.trim();

	return result;
}

//////////////////////////////////////
//			TIME METHODS			//
//////////////////////////////////////

bool GSMSim::timeSetServer(int timezone)
{
	this->print("AT+CNTPCID=1\r");
	_buffer = _readSerial();

	int time = 0;
	if (timezone <= -12)
	{
		time = -47;
	}
	if (timezone > 12)
	{
		time = 48;
	}
	if (timezone > -12 || timezone <= 12)
	{
		time = timezone * 4;
	}

	this->print(F("AT+CNTP=\"pool.ntp.org\","));
	this->print(time);
	this->print(F("\r"));

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
		return true;

	return false;
}
bool GSMSim::timeSetServer(int timezone, String server)
{
	this->print("AT+CNTPCID=1\r");
	_buffer = _readSerial();

	int time = 0;
	if (timezone <= -12)
	{
		time = -47;
	}
	if (timezone > 12)
	{
		time = 48;
	}
	if (timezone > -12 || timezone <= 12)
	{
		time = timezone * 4;
	}

	this->print(F("AT+CNTP=\""));
	this->print(server);
	this->print(F("\","));
	this->print(time);
	this->print(F("\r"));

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
		return true;

	return false;
}
String GSMSim::timeSyncFromServer()
{

	this->print(F("AT+CNTP\r"));
	_buffer = _readSerial();
	//delay(50);
	_buffer = _readSerial(20000);

	if (_buffer.indexOf("+CNTP:") == -1)
		return "AT_COMMAND_ERROR";

	String code = _buffer.substring(8);
	code.trim();

	if (code == "1")
	{
		return "TIME_SYNCHRONIZED_SUCCESS";
	}
	else if (code == "61")
	{
		return "NETWORK_ERROR";
	}
	else if (code == "62")
	{
		return "DNS_ERROR";
	}
	else if (code == "63")
	{
		return "CONNECTION_ERROR";
	}
	else if (code == "64")
	{
		return "SERVICE_RESPONSE_ERROR";
	}
	else if (code == "65")
	{
		return "SERVICE_RESPONSE_TIMEOUT";
	}
	else
	{
		return "UNKNOWN_ERROR_" + code;
	}
}

String GSMSim::timeGetRaw()
{
	this->print("AT+CCLK?\r");
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		String time = _buffer.substring(_buffer.indexOf("\"") + 1, _buffer.lastIndexOf("\""));
		return time;
	}

	return "ERROR:NOT_GET_DATETIME";
}

void GSMSim::timeGet(int *day, int *month, int *year, int *hour, int *minute, int *second)
{
	this->print("AT+CCLK?\r");
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
	{
		_buffer = _buffer.substring(_buffer.indexOf("\"") + 1, _buffer.lastIndexOf("\"") - 1);
		*year = (_buffer.substring(0, 2).toInt()) + 2000;
		*month = _buffer.substring(3, 5).toInt();
		*day = _buffer.substring(6, 8).toInt();
		*hour = _buffer.substring(9, 11).toInt();
		*minute = _buffer.substring(12, 14).toInt();
		*second = _buffer.substring(15, 17).toInt();
	}
}

//////////////////////////////////////
//			EMAIL METHODS			//
//////////////////////////////////////

bool GSMSim::emailSMTPConf(String server, String port, bool useSSL)
{
	int ssl = useSSL == true ? 1 : 0;

	this->print(F("AT+EMAILSSL="));
	this->print(ssl);
	this->print(F("\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return false;

	this->print(F("AT+EMAILCID=1\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return false;

	this->print(F("AT+EMAILTO=30\r"));
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return false;

	this->print(F("AT+SMTPSRV=\""));
	this->print(server);
	this->print(F("\",\""));
	this->print(port);
	this->print("\"\r");

	_buffer = _readSerial();

	if (_buffer.indexOf("OK") != -1)
		return true;

	return false;
}
bool GSMSim::emailSMTPAuth(String username, String password)
{
	this->print(F("AT+SMTPAUTH=1,\""));
	this->print(username);
	this->print(F("\",\""));
	this->print(password);
	this->print(F("\"\r"));

	_buffer = _readSerial();
	if (_buffer.indexOf("OK") != -1)
		return true;

	return false;
}

bool GSMSim::emailSMTPAuth(String username, String password, bool requireAuth)
{
	int auth = requireAuth == true ? 1 : 0;
	this->print(F("AT+SMTPAUTH="));
	this->print(auth);
	this->print(F(",\""));
	this->print(username);
	this->print(F("\",\""));
	this->print(password);
	this->print(F("\"\r"));

	_buffer = _readSerial();
	if (_buffer.indexOf("OK") != -1)
		return true;

	return false;
}

bool GSMSim::emailSMTPGmail(String username, String password)
{
	bool conf = emailSMTPConf("smtp.gmail.com", "465", true);
	if (conf)
		return emailSMTPAuth(username, password);

	return false;
}

String GSMSim::emailSMTPWrite(String from, String to, String title, String message)
{

	this->print(F("AT+SMTPFROM=\""));
	this->print(from);
	this->print("\"\r");
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return "ERROR:FROM_NOT_SET";

	this->print("AT+SMTPRCPT=0\r");
	_buffer = _readSerial();
	delay(50);
	this->print(F("AT+SMTPRCPT=0,1,\""));
	this->print(to);
	this->print("\"\r");
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return "ERROR:TO_NOT_SET";

	this->print(F("AT+SMTPSUB=\""));
	this->print(title);
	this->print("\"\r");
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return "ERROR:TITLE_NOT_SET";

	uint8_t length = message.length();
	this->print(F("AT+SMTPBODY="));
	this->print(length);
	this->print("\r");

	delay(50);

	this->print(message);
	this->print(F("\""));
	_buffer += _readSerial();

	if (_buffer.indexOf("OK") != -1)
		return "OK";

	return "ERROR:BODY_NOT_SET";
}

String GSMSim::emailSMTPWrite(String from, String to, String title, String message, String fromName, String toName)
{
	this->print(F("AT+SMTPFROM=\""));
	this->print(from);
	this->print("\",\"");
	this->print(fromName);
	this->print("\"\r");
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return "ERROR:FROM_NOT_SET";

	this->print("AT+SMTPRCPT=0\r");
	_buffer = _readSerial();
	delay(50);
	this->print(F("AT+SMTPRCPT=0,1,\""));
	this->print(to);
	this->print("\",\"");
	this->print(toName);
	this->print("\"\r");
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return "ERROR:TO_NOT_SET";

	this->print(F("AT+SMTPSUB=\""));
	this->print(title);
	this->print("\"\r");
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return "ERROR:TITLE_NOT_SET";

	uint8_t length = message.length();
	this->print(F("AT+SMTPBODY="));
	this->print(length);
	this->print("\r");

	delay(50);

	this->print(message);
	this->print(F("\""));
	_buffer += _readSerial();

	if (_buffer.indexOf("OK") != -1)
		return "OK";

	return "ERROR:BODY_NOT_SET";
}

String GSMSim::emailSMTPSend()
{

	this->print("AT+SMTPSEND\r");
	_buffer = _readSerial();

	if (_buffer.indexOf("OK") == -1)
		return "ERROR:EMAIL_SENDING_ERROR";

	delay(50);
	_buffer = _readSerial(30000);
	if (_buffer.indexOf("+SMTPSEND:") != -1)
	{
		String code = _buffer.substring(12);
		code.trim();

		if (code == "1")
		{
			return "SUCCESS:EMAIL_SEND";
		}
		else if (code == "61")
		{
			return "ERROR:NETWORK_ERROR";
		}
		else if (code == "62")
		{
			return "ERROR:DNS_RESOLVE_ERROR";
		}
		else if (code == "63")
		{
			return "ERROR:TCP_CONNECTION_ERROR";
		}
		else if (code == "64")
		{
			return "ERROR:TIMEOUT_SMTP_RESPONSE";
		}
		else if (code == "65")
		{
			return "ERROR:SMTP_RESPONSE_ERROR";
		}
		else if (code == "66")
		{
			return "ERROR:NOT_AUTH";
		}
		else if (code == "67")
		{
			return "ERROR:AUTH_FAILED";
		}
		else if (code == "68")
		{
			return "ERROR:BAD_RECIPIENT";
		}
		else
		{
			return "ERROR:ERROR_NO_" + code;
		}
	}
	else
	{
		delay(50);
		_buffer = _readSerial(30000);

		if (_buffer.indexOf("+SMTPSEND:") == -1)
			return "ERROR:EMAIL_TIMEOUT_ERROR";

		String code = _buffer.substring(12);
		code.trim();

		if (code == "1")
		{
			return "SUCCESS:EMAIL_SEND";
		}
		else if (code == "61")
		{
			return "ERROR:NETWORK_ERROR";
		}
		else if (code == "62")
		{
			return "ERROR:DNS_RESOLVE_ERROR";
		}
		else if (code == "63")
		{
			return "ERROR:TCP_CONNECTION_ERROR";
		}
		else if (code == "64")
		{
			return "ERROR:TIMEOUT_SMTP_RESPONSE";
		}
		else if (code == "65")
		{
			return "ERROR:SMTP_RESPONSE_ERROR";
		}
		else if (code == "66")
		{
			return "ERROR:NOT_AUTH";
		}
		else if (code == "67")
		{
			return "ERROR:AUTH_FAILED";
		}
		else if (code == "68")
		{
			return "ERROR:BAD_RECIPIENT";
		}
		else
		{
			return "ERROR:ERROR_NO_" + code;
		}
	}
}

//////////////////////////////////////
//			PRIVATE METHODS			//
//////////////////////////////////////

// READ FROM SERIAL
String GSMSim::_readSerial()
{

	uint64_t timeOld = millis();

	while (!this->available() && !(millis() > timeOld + TIME_OUT_READ_SERIAL))
	{
		delay(13);
	}

	String str = "";

	while (this->available())
	{
		if (this->available())
		{
			str += (char)this->read();
		}
	}

	return str;
}

String GSMSim::_readSerial(uint32_t timeout)
{

	uint64_t timeOld = millis();

	while (!this->available() && !(millis() > timeOld + timeout))
	{
		delay(13);
	}

	String str = "";

	while (this->available())
	{
		if (this->available())
		{
			str += (char)this->read();
		}
	}

	return str;
}
