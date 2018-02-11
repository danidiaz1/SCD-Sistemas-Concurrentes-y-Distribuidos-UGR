// Compilar con: g++ -std=c++11 -I. -o fumadores Semaphore.cpp fumadores.cpp -pthread

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <string>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

string ingredientes[3] = {"Tabaco", "Papel", "Cerillas"}; // Ingrediente que necesita cada fumador
Semaphore mostr_vacio = 1;
Semaphore ingr_disp[3] = {0,0,0};


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

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
	while (true)
	{
		int i = aleatorio<0,2>();
		mostr_vacio.sem_wait();
		cout << "Puesto ingrediente: " << ingredientes[i] << endl;
		ingr_disp[i].sem_signal();
	}
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente " << ingredientes[num_fumador] << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
	cout << "Fumador " << num_fumador << " esperando ingrediente " << ingredientes[num_fumador] << endl;
	while( true )
	{	
		ingr_disp[num_fumador].sem_wait();
		cout << "Retirado ingrediente: " << ingredientes[num_fumador] << " por el fumador " << num_fumador << endl;
		mostr_vacio.sem_signal();
		fumar(num_fumador);
	}
}

//----------------------------------------------------------------------

int main()
{
   thread fumador0 ( funcion_hebra_fumador, 0 ),
          fumador1 ( funcion_hebra_fumador, 1 ),
          fumador2 ( funcion_hebra_fumador, 2 ),
          estanquero ( funcion_hebra_estanquero );

   fumador0.join();
   fumador1.join();
   fumador2.join();
   estanquero.join();
}
