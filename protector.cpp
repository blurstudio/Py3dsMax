/*
	\file		protector.cpp

	\remarks	Implementation of the Protector class

	\author		Blur Studio (c) 2010
	\email		beta@blur.com

	\license	This software is released under the GNU General Public License.  For more info, visit: http://www.gnu.org/
*/

#include "imports.h"

#include <map>

#include "macros.h"
#include "wrapper.h"
#include "protector.h"

// Create the staic protection list
typedef std::map<long,PyObject*>			ProtectionMap;				// using a hash map vs. a list for speed (initial version used a list and took forever) - EKH 2011
typedef std::pair<long,PyObject*>			ProtectionPair;
typedef std::map<long,PyObject*>::iterator	ProtectionIterator;

ProtectionMap* protected_objects = 0;

visible_class_instance( Protector, "PyMemProtector" );
Value* ProtectorClass::apply( Value** arg_list, int count, CallContext* cc ) {
	one_value_local( result );
	vl.result = new Protector();
	return_value( vl.result );
}

Protector::Protector() {}
Protector::~Protector() {}

void Protector::gc_trace() {
	// Step 1: trace this item
	Value::gc_trace();

	if ( protected_objects ) {
		for ( ProtectionIterator it = protected_objects->begin(); it != protected_objects->end(); ++it )
			ObjectWrapper::gc_protect( (*it).second );
	}
}

Value* Protector::get_property( Value** arg_list, int count ) {
	// return the length of the memory list
	if ( arg_list[0] == n_count ) {
		if ( protected_objects )
			return Integer::intern( protected_objects->size() );
		return Integer::intern(0);
	}
	// dump the contents of the memory list
	else if ( arg_list[0] == Name::intern( "dump" ) ) {
		for ( ProtectionIterator it = protected_objects->begin(); it != protected_objects->end(); ++it ) {
			ObjectWrapper::log( (*it).second );
		}
		return &ok;
	}

	// return default value results for the keywords
	return Value::get_property( arg_list, count );
}

void Protector::protect( PyObject* obj ) {
	// Step 1: insert the object into the protection list
	long hash = PyObject_Hash(obj);
	protected_objects->insert( ProtectionMap::value_type( hash, obj ) );
}

void Protector::unprotect( PyObject* obj ) {
	// Step 1: remove the protected object
	if ( protected_objects ) {
		long hash = PyObject_Hash(obj);
		protected_objects->erase( protected_objects->find( hash ) );
	}
}

void Protector::init() {
	// Step 1: create the array if it does not exist
	if ( !protected_objects )
		protected_objects = new ProtectionMap();
}