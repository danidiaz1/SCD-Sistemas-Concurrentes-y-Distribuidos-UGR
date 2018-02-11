// Compilar con:
// g++ -std=c++11 -pthread -o barbero barbero.cpp HoareMonitor.cpp
// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 2. Problema del barbero durmiente.
//
// archivo: barbero.cpp
//
// Autor: Daniel Díaz Pareja
// -----------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <random>
#include "HoareMonitor.hpp"
#include <string>
#include <mutex>

using namespace std ;
using namespace HM ;

mutex mtx; // mutex de escritura en pantalla
const int CLIENTES = 10;
//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// *****************************************************************************
// clase para el monitro de la barbería, semántica SU

class Barberia : public HoareMonitor
{
   private:
   CondVar barbero; // Cola en la que duerme el barbero
   CondVar sala_espera; // cola en la que esperan las hebras si la silla
   						// está ocupada las hebras.
   CondVar silla; // Cola que representa a si hay alguien pelandose

   public:
   Barberia( ) ; // constructor
   void cortarPelo(int i);	
   void siguienteCliente();
   void finCliente();
   int num_clientes;
};
// -----------------------------------------------------------------------------

Barberia::Barberia()
{
   sala_espera = newCondVar();
   barbero = newCondVar();
   silla = newCondVar();
   num_clientes = 0;
}

// -----------------------------------------------------------------------------

void Barberia::siguienteCliente()
{
	if (num_clientes == 0){ // Si no hay nadie a quien pelar, esperamos
		cout << endl << "El barbero esta durmiendo" << endl;
		barbero.wait(); // El barbero, por defecto, siempre está dormido a no ser que
						// lo despierte alguien
	}

	if (silla.empty()){ // Si no se está pelando a nadie
		cout << endl << "El barbero llama a un cliente" << endl;
		sala_espera.signal(); // Cuando se despierte al barbero, se llama a un cliente
	}
}

// -----------------------------------------------------------------------------

void Barberia::finCliente()
{
	cout << endl << "El barbero termina de pelar al cliente" << endl;
	silla.signal();
	num_clientes--; // Cuando se termine de pelar, hay un cliente menos.
}

void Barberia::cortarPelo(int i)
{

	num_clientes++; // Entra un nuevo cliente a la barbería
	cout << endl << "El cliente " << i << " entra en la barbería." << endl;

	// Si el barbero está durmiendo
	if (!barbero.empty()){
		cout << endl << "El cliente " << i << " despierta al barbero " << endl;
		barbero.signal();
	}
	
	if (num_clientes > 1){ // Si hay alguien pelándose o en cola además de mi, esperamos
		cout << endl << "El cliente " << i << " acaba de entrar en sala de espera" << endl;
		sala_espera.wait(); // Esperamos en la sala de espera
	}

	cout << endl << "El cliente " << i << " entra a pelarse." << endl; 
	silla.wait(); // Se nos está pelando, esperamos a que el barbero termine
	cout << endl << "El cliente " << i << " se marcha de la barbería. " << endl;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CortarPeloACliente()
{
	// calcular milisegundos aleatorios de duración de la acción de cortar el pelo al cliente
   chrono::milliseconds duracion_pelar( aleatorio<20,200>() );

   // espera bloqueada un tiempo igual a 'duracion_pelar' milisegundos
   this_thread::sleep_for( duracion_pelar );

   cout << "El barbero pela al cliente y tarda: (" << duracion_pelar.count() << " milisegundos)." << endl;
}

//-------------------------------------------------------------------------
// Función que simula la acción de esperar, como un retardo aleatoria de la hebra
void EsperarFueraBarberia( int i )
{ 
   // calcular milisegundos aleatorios de duración de esperar fuera de la barbería
   chrono::milliseconds duracion_espera( aleatorio<20,200>() );

   // informa de que comienza a esperar

   mtx.lock();
    cout << "Cliente " << i << "  :"
          << " espera fuera (" << duracion_espera.count() << " milisegundos)" << endl;
   mtx.unlock();
   // espera bloqueada un tiempo igual a 'duracion_espera' milisegundos
   this_thread::sleep_for( duracion_espera );

   // informa de que ha terminado de esperar
    cout << "Cliente " << i << "  : termina de esperar fuera." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del cliente
void funcion_hebra_cliente( MRef<Barberia> barberia, int i )
{
	while( true )
	{	
		barberia->cortarPelo(i);
		EsperarFueraBarberia(i);
	}
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del barbero
void funcion_hebra_barbero(MRef<Barberia> barberia)
{
	while (true)
	{
		barberia->siguienteCliente();
    	CortarPeloACliente();
    	barberia->finCliente();
	}

}
// *****************************************************************************

int main()
{
   cout <<  "Barberia SU: inicio simulación." << endl ;

   // crear monitor  (es una referencia al mismo, de tipo MRef<...>)
   MRef<Barberia> barberia = Create<Barberia>( );

   // crear y lanzar hebras
   thread barbero ( funcion_hebra_barbero, barberia );
   thread clientes[CLIENTES];

   for (int i = 0; i < CLIENTES; i++)
      clientes[i] = thread(funcion_hebra_cliente, barberia, i);

   for (int i = 0; i < CLIENTES; i++)
      clientes[i].join();

   barbero.join();
}