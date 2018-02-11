// Compilar con:
// g++ -std=c++11 -pthread -o fumadores fumadores.cpp HoareMonitor.cpp
// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 2. Problema de los fumadores.
//
// archivo: fumadores.cpp
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

string ingredientes[3] = {"Tabaco", "Papel", "Cerillas"}; // Ingrediente que necesita cada fumador
mutex mtx; // mutex de escritura en pantalla
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
// clase para el monitro de los fumadores, semántica SU

class Estanco : public HoareMonitor
{
   private:

   static const int NUM_FUMADORES = 3;
   //bool mostrador_tiene_ingrediente;
   int ingrediente_en_mesa; // Ingrediente que está actualmente en la mesa
   							// Si vale -1, la mesa está vacía

   CondVar mostrador_vacio;       // cola en la que espera el estanquero si
   								  // el mostrador está lleno
   CondVar ingr_disp[NUM_FUMADORES]; // Una cola para cada fumador.

   public:
   Estanco( ) ; // constructor
   void ponerIngrediente(int ingr);
   void obtenerIngrediente(int num_fumador);
   void esperarRecogidaIngrediente();
};
// -----------------------------------------------------------------------------

Estanco::Estanco()
{
   //mostrador_tiene_ingrediente = false;
   ingrediente_en_mesa = -1;
   mostrador_vacio = newCondVar();

   for (int i = 0; i < NUM_FUMADORES; i++)
   		ingr_disp[i] = newCondVar();
}

// -----------------------------------------------------------------------------

void Estanco::ponerIngrediente(int ingr)
{
	
	ingrediente_en_mesa = ingr;
	cout << "Puesto ingrediente: " << ingredientes[ingr] << endl;
	//mostrador_tiene_ingrediente = true;
	ingr_disp[ingr].signal(); // Se desbloquea al fumador que necesita el ingrediente
}

// -----------------------------------------------------------------------------

void Estanco::obtenerIngrediente(int numero_fumador)
{
	if (ingrediente_en_mesa != numero_fumador)
		ingr_disp[numero_fumador].wait();

	ingrediente_en_mesa = -1;

	mostrador_vacio.signal();
}

void Estanco::esperarRecogidaIngrediente()
{
	if (ingrediente_en_mesa != -1)
		mostrador_vacio.wait();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

int ProducirIngrediente()
{
	int ingr = aleatorio<0,2>();
	// calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_producir( aleatorio<20,200>() );


   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_producir );

   cout << "Estanquero produce ingrediente: " << ingredientes[ingr] << 
   ", y tarda: " << duracion_producir.count() << " milisegundos	" << endl;

	return (ingr);
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra
void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

   mtx.lock();
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
   mtx.unlock();
   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente " << ingredientes[num_fumador] << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void funcion_hebra_fumador( MRef<Estanco> estanco, int num_fumador )
{
	cout << "Fumador " << num_fumador << " esperando ingrediente " << ingredientes[num_fumador] << endl;
	while( true )
	{	
		estanco->obtenerIngrediente(num_fumador);
		cout << "Retirado ingrediente: " << ingredientes[num_fumador] << " por el fumador " << num_fumador << endl;
		fumar(num_fumador);
	}
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void funcion_hebra_estanquero(MRef<Estanco> estanco)
{
	while (true)
	{
		int ingr = ProducirIngrediente();
		estanco->ponerIngrediente(ingr);
		estanco->esperarRecogidaIngrediente();
	}

}
// *****************************************************************************

int main()
{
   cout <<  "Fumadores SU: inicio simulación." << endl ;

   // crear monitor  (es una referencia al mismo, de tipo MRef<...>)
   MRef<Estanco> estanco = Create<Estanco>( );

   // crear y lanzar hebras
   thread fumador0 ( funcion_hebra_fumador, estanco, 0 ),
          fumador1 ( funcion_hebra_fumador, estanco, 1 ),
          fumador2 ( funcion_hebra_fumador, estanco, 2 ),
          estanquero ( funcion_hebra_estanquero, estanco );

   fumador0.join();
   fumador1.join();
   fumador2.join();
   estanquero.join();
}