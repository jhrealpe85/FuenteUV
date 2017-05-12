/* ============================================================================
  Autor:			  Jhon Jairo Realpe
  Proyecto/Archivo:               FuenteUV/main.ino
  Microcontrolador:               atmega328p
  Frecuencia:                     16MHz
  Licencia:                       Creative Commons
  Fecha última Compilación:       Ago. 20/16
  ===============================================================================
  Descripción:                    Este archivo muestra la Implementación  e
                                  integración de los módulos de funcionamiento
                                  de la fuente UV

  Historia:                       Implementación módulos de escritura con el DAC
                                  Implementación módulo de Potencia
                                  Implementación módulo EEPROM
                                  Implementación módulos Ingreso datos
                                  Implementación módulos Muestra datos
                                  Integración de todos los módulos Ago. 20/16
  =============================================================================*/
// Declaración de librerías     
#include <LiquidCrystal.h>				// Librería usada para controlar LCD   
#include <Keypad.h>								// Librería usada para manipular 
#include <SPI.h>									// Liberia SPI para controlar LCD
#include <EEPROM.h>								// Librería usada para almacenar datos en la EEPROM

#define DATA  4										// Definición de puertos para la comunicación por SPI 
#define CLOCK 3										// con el DAC (Conversor digital a análogo)
#define CS 2	

#define HALF_CLOCK_PERIOD 10			// Definición del periodo de tiempo para comunicación 	
																	// por SPI con el DAC

#define DIRECCIONINT 0						// Definición de posiciones de memoria en bytes para 
#define DIRECCIONFLO 2						// almacenamiento de información en la EEPROM

LiquidCrystal lcd(10);						// Uso de la función LCD modificada de librería LiquidCrystal (SPI)
																	// para controlar por pin10

const int ROWS = 4; 							// Número de Filas Teclado
const int COLS = 3; 							// Número de Columnas Teclado

char keys[ROWS][COLS] = { 				// keymap define el carácter retornado cuando se presione una tecla
 { '1', '2', '3' } ,
 { '4', '5', '6' } ,
 { '7', '8', '9' } ,
 { '.', '0', '#' }
};

byte rowPins[ROWS] = { 1, 0, 18, 19 }; // Asignación de pines del microcontrolador para definir
byte colPins[COLS] = { 15, 16, 17 };   // las columnas 0 a 2 y filas 0 a 3 del teclado matricial

// Definición de objeto Keypad con los atributos Citados anteriormente
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// Definición de constantes curva característica (%Intensidad vs. Corriente) del LED UV
double yo, Ao, Ro;								
// Definición de constantes curva característica (Corriente Vs. No de Bits) 
// del Driver del arreglo de LEDs UV
double B, M;

unsigned long tdelay;							// Periodo de retardo tiempo de exposición

// Definición de variables de control de potencia y tiempo de exposición de la fuente UV
int pw; float time;								

int countinfo;										// Definición de variable de conteo de instrucciones para mostrar información por LCD

/* ============================================================================ */
// Inicio Función setup
void setup(){
	// Se definen los puertos del microcontrolador  para la comunicación con el DAC como salidas
	pinMode(DATA, OUTPUT);					
	pinMode(CLOCK,OUTPUT);
	pinMode(CS,OUTPUT);
	
	writeValue(0);								// Se llama a la función write para inicializar el DAC en cero
	
	// Se inicializan las constantes de la curva característica del LED UV
	yo = 128.5358;
	Ao = -137.74992;
	Ro = -0.08397;
	
	// Se inicializan las constantes de la curva característica del Driver del arreglo de LEDS
	B = -4.01533;
	M = 1.95876;
	
	tdelay = 100000;							// Se define el periodo de retardo a 100ms
	
	countinfo = 0;
	
	lcd.begin(16, 2);							// Se define LCD de 16 columnas x 2 filas
	lcd.clear();									// Limpia pantalla de LCD
	lcd.setCursor(2, 0); lcd.print("**Welcome**");	// Escribe mensaje de bienvenida en LCD
	delay(2000);
}
// Fin Función setup
/* ============================================================================ */

/* ============================================================================ */
// Inicio Función Principal
void loop(){
	int inicio;	
	lcd.clear();	// Limpia pantalla LCD
	// Menú de selección opciones Nuevo o Repetir
	lcd.setCursor(0, 0); lcd.print("1:New"); 
	lcd.setCursor(7, 0); lcd.print("2:Repeat"); 
	// Lee Opción desde teclado matricial
	inicio = keypad2int(1, 8, 1);
	delay(2000);	
	switch (inicio) {
		case 1: {Nuevo();} break;		// se Llama a la función nuevo
		case 2: {Repetir();} break;	// se Llama a la función repetir
		default: {}      
	}	
}
// Fin Función Principal
/* ============================================================================ */

/* ============================================================================ */
// Inicio Definición de las funciones Nuevo y repetir
void Nuevo(){
	int inicio;
	initNuevo:
	Modificar();
	contiNuevo:
	lcd.clear();
	// Menú de selección opciones iniciar, modificar o cancelar
	lcd.setCursor(0, 0); lcd.print("1:Start"); 
	lcd.setCursor(8, 0); lcd.print("2:Modify"); 
	lcd.setCursor(0, 1); lcd.print("3:Cancel:");
	// Lee Opción desde teclado matricial
	inicio = keypad2int(1, 9, 1);
	delay(2000);	
	switch (inicio) {
		case 1: {Iniciar();} break;
		case 2: {goto initNuevo;} break;
		case 3: {} break;
		default: {goto contiNuevo;} break;      
	}	
}

void Repetir(){
	int inicio;	
	// Accede a los valores de Potencia y tiempo de exposición
	// almacenados en la EEPROM
	pw = readIntEeprom(DIRECCIONINT);
	time = readFloatEeprom(DIRECCIONFLO);
	lcd.clear();
	// Muestra los valores en la pantalla LCD
	lcd.setCursor(0,0); lcd.print("Power=");
	lcd.setCursor(6,0); lcd.print(pw); 
	lcd.setCursor(8,0); lcd.print("mW/cm^2");	
	lcd.setCursor(0,1); lcd.print("Exp.Dur=");
	lcd.setCursor(8,1); lcd.print(time); 
	lcd.setCursor(12,1); lcd.print("s");
	delay(3000);	
	lcd.clear();
	// Menú de selección opciones iniciar o cancelar
	lcd.setCursor(0, 0); lcd.print("1:Start"); 	
	lcd.setCursor(0, 1); lcd.print("2:Cancel:");
	inicio = keypad2int(1, 9, 1);
	delay(2000);	
	switch (inicio) {
		case 1: {Iniciar();} break;		
		case 3: {} break;
		default: {}      
	}
}
// Fin Definición de las funciones Nuevo y repetir
/* ============================================================================ */

/* ============================================================================ */
// Inicio Definición de las subfunciones de función Nuevo: Iniciar y modificar
void Iniciar(){
	lcd.clear(); 
	// Llama a la función de control de potencia y tiempo de exposición
	powtimeUv(pw, time);
	// Terminado el proceso escribe un valor de cero en el driver de la matriz de LEDs
	writeValue(0);
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Finished"); 
	lcd.setCursor(0, 1); lcd.print("Exposing...");
	delay(1000);
}

void Modificar(){
	contModiPw:
	lcd.clear(); 
	// Menú de selección de opciones para modificar la
	// potencia y tiempo de exposición
	lcd.setCursor(0,0); lcd.print("Enter 2-digit");
	lcd.setCursor(0,1); lcd.print("Power=");
	pw = keypad2int(2, 6, 1);
	lcd.setCursor(8,1); lcd.print("mW/cm^2");	
	delay(2000);
	// Se valida el valor máximo ingresado de potencia a 20mw/cm^2
	if (pw>20){
		lcd.clear(); 
		lcd.setCursor(0,0); lcd.print("Attention!");
		lcd.setCursor(0,1); lcd.print("Power<=20mW/cm^2");
		delay(2000);
		goto contModiPw;
	}
	contModiTime:
	lcd.clear();
	lcd.setCursor(0,0); lcd.print("Enter Exposure");
	lcd.setCursor(0,1); lcd.print("Duration=");
	time = keypad2dec(4, 9, 1);
	lcd.setCursor(13,1); lcd.print("s");	
	delay(2000);
	// Se valida el valor máximo ingresado de tiempo de exposición a 60s
	if (time>60){
		lcd.clear(); 
		lcd.setCursor(0,0); lcd.print("Attention!");
		lcd.setCursor(0,1); lcd.print("Time<=60s");
		delay(2000);
		goto contModiTime;
	}
	lcd.clear();
	lcd.setCursor(0,0); lcd.print("Power=");
	lcd.setCursor(6,0); lcd.print(pw); 
	lcd.setCursor(8,0); lcd.print("mW/cm^2");	
	lcd.setCursor(0,1); lcd.print("Exp.Dur=");
	lcd.setCursor(8,1); lcd.print(time); 
	lcd.setCursor(12,1); lcd.print("s");
	delay(3000);	
	// Se almacenan la potencia y tiempo de exposición en la EEPROM
	writeIntEeprom(pw,DIRECCIONINT);
	writeFloatEeprom(time,DIRECCIONFLO);
}
// Fin Definición de las subfunciones de función Nuevo: Iniciar y modificar
/* ============================================================================ */

/* ============================================================================ */
// Inicio Definición de las funciones de control de potencia y tiempo de exposición 
// y escritura con el DAC
void powtimeUv(int value, float t){	
	unsigned long prevms = 0; 
	double Nbit, Id;
	int P;
	while(t > 0){
		// Escalamiento de la potencia ingresada al 100% en Intensidad
		P = map(value,0,20,0,80);
		// Se obtiene el valor de corriente necesario para la potencia deseada
		// a partir de la función de la curva %Intensidad Vs. Corriente en un LED
		Id = 20*log((P - yo)/Ao)/Ro;  
		// Se calcular el Nbits necesarios en el DAC para el valor de 
		// corriente deseado a partir de la función de la curva Nbit Vs. Corriente en el arreglo de LEDs
		Nbit = (Id - B)/M;
		// Se llama a la función writeValue que escribe el Nbit calculado en el DAC
		writeValue((int)Nbit);			
		// Se llama a la función mostrar información
		mostrarinfo(t); 
		// Se genera un conteo regresivo a partir del valor ingresado
		// como tiempo de exposición
		if((unsigned long)(micros()-prevms)>=tdelay){ 
			prevms = micros();
			t -= 0.1;						
		}  
    }   
}

void writeValue(int value) {
	// Configura los puertos para iniciar la comunicación de datos
	digitalWrite(CS,HIGH);
	digitalWrite(CS,LOW);		
	digitalWrite(CLOCK,LOW);
	value = value << 2; // Añade 2 bits cero al final de la cadena de datos
	// A La cadena de datos de 10 bits se debe añadir 2 bits cero 
	// en el bit menos significativo dado que la entrada del DAC es de 12 bits 
	// (Ver Datasheet TLC5615C)
	for(int i=11; i>=0; i--) {		//Envía los 12 bits de datos
		// Escribe los datos haciendo un corrimiento en i-ésima posición
		digitalWrite(DATA, (value & (1 << i)) >> i );	
		// Se genera un tren de pulsos (frecuencia de reloj) 
		delayMicroseconds(HALF_CLOCK_PERIOD);
		digitalWrite(CLOCK,HIGH);// DAC recibe datos en el flanco de subida
		delayMicroseconds(HALF_CLOCK_PERIOD);
		digitalWrite(CLOCK,LOW);
	}
	digitalWrite(CS,HIGH);//Finaliza la envío de la secuencia de datos de 12 bit
}
// Fin Definición de las funciones de control de potencia y tiempo de exposición 
// y escritura con el DAC
/* ============================================================================ */

/* ============================================================================ */
// Inicio de módulos para ingresar y mostrar información
int keypad2int(int n, int c, int f){
	char Cadkey[n],key;
	int char2int;  
	for(int i=0;i<n;i++){
		key=keypad.waitForKey();          
		Cadkey[i]=key;
		lcd.setCursor(c,f);     
		lcd.print(key);
		c++;          
	}
	// Convierte las caracteres leídos en el teclado matricial
	// a un numero entero
	char2int = atoi(Cadkey);
	return(char2int); 
}

float keypad2dec(int n, int c, int f){
	char key;
	int char2dec=0, factor=100;  
	for(int i=0;i<n;i++){
		key=keypad.waitForKey();     
	// Convierte las caracteres leídos en el teclado matricial
	// a un numero flotante		
		if (key != '.'){
			char2dec += ((int)key - 48)*factor;
			factor /= 10;			
		}
		lcd.setCursor(c,f);     
		lcd.print(key);
		c++;          
	}
	return((float)char2dec/10.0); 
}

// Se muestra los parámetros de exposición en la pantalla LCD
void mostrarinfo(float t){
	if (countinfo > 2500) {   
		lcd.setCursor(0, 0); lcd.print("Exposing...");
		lcd.setCursor(0, 1); lcd.print("Count Down:");
		lcd.setCursor(11, 1); lcd.print(t);
		lcd.setCursor(15, 1); lcd.print("s");
    countinfo = 0;
  } else {
    countinfo++;
  }	
}
// Fin de módulos para ingresar y mostrar información
/* ============================================================================ */

/* ============================================================================ */
// Inicio de módulos de lectura y escritura de información en la EEPROM
void writeIntEeprom(int valori, int direccion){		// Almacena datos tipo entero en la EEPROM
	byte lowByte = ((valori >> 0) & 0xFF);
	byte highByte = ((valori >> 8) & 0xFF);
	EEPROM.write(direccion, lowByte);
	EEPROM.write(direccion+ 1, highByte);
}

int readIntEeprom(int direccion){		// Accede a los datos tipo entero en la EEPROM
  byte lowByte = EEPROM.read(direccion);
	byte highByte = EEPROM.read(direccion + 1);
	return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

void writeFloatEeprom(float valorf, int direccion){		// Almacena datos tipo flotante en la EEPROM
	union float_union{
		byte unionByte[4];
		float unionFloat;
	}resultado;
	
	resultado.unionFloat = valorf;
	for (int i = 0; i < 4; i++) {
		EEPROM.write(direccion + i, resultado.unionByte[i]);
	}
}

float readFloatEeprom(int direccion){		// Accede a los datos tipo flotante en la EEPROM
	union float_union {
		byte unionByte[4];
		float unionFloat;
	}resultado;
	
	for (int i = 0; i < 4; i++) {
		resultado.unionByte[i] = EEPROM.read(direccion + i);
	}
	return resultado.unionFloat;	
}
// Fin de módulos de lectura y escritura de información en la EEPROM
/* ============================================================================ */
