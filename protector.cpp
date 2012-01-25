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

visible_class_instance( Protector, "PyMemProtector" );

int Protector::mCount = 0;
ValueWrapper * Protector::mFirst = 0;

Protector::Protector()
{}

Protector::~Protector()
{}

Value* ProtectorClass::apply( Value** arg_list, int count, CallContext* cc )
{
	one_value_local( result );
	vl.result = new Protector();
	return_value( vl.result );
}

void Protector::gc_trace() {
	// Step 1: trace this item
	Value::gc_trace();

	ValueWrapper * cur = mFirst;
	while( cur ) {
		cur->mValue->gc_trace();
		cur = cur->mNext;
	}
}

Value* Protector::get_property( Value** arg_list, int count ) {
	// return the length of the memory list
	if ( arg_list[0] == n_count ) {
		return Integer::intern( mCount );
	}
	// dump the contents of the memory list
	else if ( arg_list[0] == Name::intern( "dump" ) ) {
		ValueWrapper * cur = mFirst;
		while( cur ) {
			ObjectWrapper::log((PyObject*)cur);
			cur = cur->mNext;
		}
		return &ok;
	}

	// return default value results for the keywords
	return Value::get_property( arg_list, count );
}

void Protector::protect( ValueWrapper * obj ) {
	mCount++;
	obj->mNext = mFirst;
	obj->mPrev = 0;
	if( mFirst )
		mFirst->mPrev = obj;
	mFirst = obj;
}

void Protector::unprotect( ValueWrapper * obj ) {
	mCount--;
	if( obj->mNext )
		obj->mNext->mPrev = obj->mPrev;
	if( obj->mPrev )
		obj->mPrev->mNext = obj->mNext;
	if( obj == mFirst )
		mFirst = obj->mNext;
}
