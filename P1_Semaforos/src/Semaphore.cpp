// *****************************************************************************
//
// Semaphores implementation using C++11 concurrency features.
// (private implementation)
// Copyright (C) 2017  Carlos Ureña Almagro
//
// April 2017: created
// 15-Sept-2017: removed reference count, now 'std::shared_ptr' is used instead
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// *****************************************************************************

#include <iostream>
#include <cassert>
#include <Semaphore.h>

namespace SEM
{

using namespace std ;

// *****************************************************************************
// representation of semaphores (not public)

class SemaphoreRepr
{
   friend class Semaphore ;

   public:
   // create with an initial unsigned value
   SemaphoreRepr( unsigned init_value ) ;
   ~SemaphoreRepr() ;

   private:

   unsigned                  value ;  // current semaphore value
   std::mutex *              mtx ;    // for mutual exclusion in semaphore updating
   std::condition_variable * queue ;  // queue with waiting threads
   unsigned                  num_wt ; // current number of waiting threads

   unsigned num_waiting_threads() const ;  // returns number of waiting threads
   void     sem_wait();             // wait operation
   bool     sem_signal();           // signal operation (returns true if there was any waiting thread)

} ;

// *****************************************************************************
// implementation of 'Semaphore' interface

Semaphore::Semaphore( unsigned init_value )
{
   //cout << "begins  Semaphore::Semaphore( int == " << init_value << ")" << endl << flush ;
   repr = std::make_shared<SemaphoreRepr>( init_value );
   assert( repr != NULL);
   //repr->refcount = 1 ;
}
// -----------------------------------------------------------------------------

Semaphore::Semaphore( const Semaphore & sem )
{
  assert( sem.repr != nullptr );
  //cout << "begins  Semaphore::Semaphore( Semaphore &, value == " << sem.repr->value << ", ref cnt == "<< sem.repr->refcount <<" )" << endl << flush ;

  repr = sem.repr ;
}

// -----------------------------------------------------------------------------

void Semaphore::sem_wait(  )
{
   assert( repr != nullptr );
   repr->sem_wait();
}
// -----------------------------------------------------------------------------

void Semaphore::sem_signal(  )
{
   assert( repr != nullptr );
   repr->sem_signal() ;
}
// *****************************************************************************
// Implementation of (private) semaphore representation methods

SemaphoreRepr::SemaphoreRepr( unsigned init_value )
{
  cout << "begins  SemaphoreRepr::SemaphoreRepr( int == " << init_value << ")" << endl << flush ;
  mtx   = nullptr ;
  queue = nullptr ;

  mtx   = new std::mutex ;
  queue = new std::condition_variable ;

  value = init_value ;
  num_wt = 0 ;
}
// -----------------------------------------------------------------------------

unsigned SemaphoreRepr::num_waiting_threads() const
{
  return num_wt ;
}
// -----------------------------------------------------------------------------

void SemaphoreRepr::sem_wait()
{
  assert( mtx != nullptr );
  assert( queue != nullptr );

  // gain mutex
  std::unique_lock<std::mutex> lock( *mtx );

  // wait while value is cero (must be 'while' because of possible spurious wakeups )
  num_wt += 1 ;          // register new waiting thread
  while ( value == 0 )
    queue->wait( lock ); // release mutex when waiting, re-gain after
  num_wt -= 1 ;          // register that thread is no longer waiting

  // decrease value
  assert( 0 < value );
  value -= 1 ;

  // release mutex:
}
// -----------------------------------------------------------------------------

bool SemaphoreRepr::sem_signal()
{
  assert( mtx != nullptr );
  assert( queue != nullptr );

  // gain mutex
  std::unique_lock<std::mutex> lock( *mtx );

  // increase value
  value += 1 ;

  // signal one waiting thread, if any
  if ( 0 < num_wt )
  {
    queue->notify_one() ;
    return true ;
  }
  return false ;

}
// -----------------------------------------------------------------------------

SemaphoreRepr::~SemaphoreRepr()
{
  using namespace std ;
  //cout << "begins ~SemaphoreRepr( value == " << value << ") ...." << endl << flush ;
  assert( mtx != nullptr );
  assert( queue != nullptr );

  delete mtx ;
  mtx = nullptr ;

  delete queue ;
  queue = nullptr ;
}

} // end namespace SCD
