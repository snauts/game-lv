#ifndef __STATE_H__
#define __STATE_H__

#include <ostream>
#include <map>

#ifdef unix
#include <stdlib.h>
#endif

using namespace std;

#include "debug.h"
extern debug_out dout;

template<class state_type, class value_type, class output>
class state_machine {
	private:
		map<state_type, value_type> states;
	public:
		typedef typename map<state_type, value_type>::iterator SI;
		typedef typename map<state_type, value_type>::const_iterator CSI;
		state_machine() {
		}
		~state_machine() {
		}
		value_type get_value(const state_type& id);
		void set_value(const state_type& id, const value_type& val);
		bool is_value(const state_type& id) const;
		const map<state_type, value_type> &getStatesRef() const { return states; }
		void clear();
};


template<class state_type, class value_type, class output>
bool state_machine<state_type, value_type, output>::is_value(const state_type& id) const {
	return (states.find(id) != states.end());
}

template<class state_type, class value_type, class output>
void state_machine<state_type, value_type, output>::clear() {
	states.clear();
}

template<class state_type, class value_type, class output>
value_type state_machine<state_type, value_type, output>::get_value(const state_type& id) {
	if(!is_value(id)) {
		dout << "ERROR- state machine doesn't has value with id: '"
		     << id << "'" << endl;
		exit(1);
	}
	SI i = states.find(id);
	return (i->second);
}

template<class state_type, class value_type, class output>
void state_machine<state_type, value_type, output>::set_value(const state_type& id, const value_type& val) {
	if(!is_value(id)) {
		states.insert(make_pair<state_type, value_type>(id, val));
	} 
	else {
		SI i = states.find(id);
		i->second = val;
	}
}

template<class state_type, class value_type, class output>
output& operator<< (output& os,
		    const state_machine<state_type, value_type, output>& m) {
#ifndef __GNUC__
	typedef typename map<state_type, value_type>::const_iterator CSI;
	const map<state_type, value_type> &states = m.getStatesRef();
	CSI i = states.begin();

	os << "State machine:" << endl;
	if(states.empty()) {
		os << " No states" << endl;
		return os;
	}
	while(i != states.end())
	{
		os << " Id : '" << i->first
		   << "' , value : '" << i->second << "'" << endl;
		i++;
	}
#else	
	// #warning FIXMEEE, FIXMEEE
#endif
	return os;
}

#endif
