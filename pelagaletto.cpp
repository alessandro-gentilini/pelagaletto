/*
Pelagaletto
Pelagaletto is an Italian card game similar to Beggar-My-Neighbour. 

Warning: the code is not polished!

Author: Alessandro Gentilini
Source: http://code.google.com/p/pelagaletto/
License: GNU General Public License v3

$Rev$
$Date$
*/

#include <string>
#include <sstream>
#include <exception>
#include <algorithm>
#include <iostream>
#include <deque>
#include <iomanip>
#include <set>

typedef std::deque< int > deck_t;
typedef deck_t::value_type card_t;

void win( deck_t& table, deck_t& winner )
{
	while ( !table.empty() ) {
		winner.push_back( table.back() );
		table.pop_back();
	}
}

card_t put( deck_t& table, deck_t& putter )
{
	card_t f = putter.front();
	table.push_front( f );
	putter.pop_front();
	return f;
}

void myswap( deck_t** first_player, deck_t* A, deck_t* B )
{
	if ( *first_player == A ) {
		*first_player = B;
	} else if ( *first_player == B ) {
		*first_player = A;
	}
}

std::string stringify( const deck_t& d )
{
	std::ostringstream ss;
	for ( deck_t::const_iterator it = d.begin(); it != d.end(); it++ ) {
		ss << std::setw(2) << std::setfill('0') << *it << " ";
	}
	return ss.str();
}

deck_t init( const std::string& s )
{
	deck_t d;
	std::istringstream ss(s);
	card_t c;
	while( ss >> c ) {
		d.push_back( c );
	}
	return d;
}

#ifdef COMPILE_ON_WINDOWS
#include <windows.h>
ULONGLONG ptime()
{
	FILETIME c, e, k, u;
	::GetProcessTimes( ::GetCurrentProcess(), &c, &e, &k, &u );
	ULARGE_INTEGER li;
	li.HighPart = u.dwHighDateTime;
	li.LowPart = u.dwLowDateTime;
	return li.QuadPart;
}
#endif

std::string game( const deck_t& deck )
{
	deck_t A( deck.begin(), deck.begin()+deck.size()/2 );
	deck_t B( deck.begin()+deck.size()/2, deck.end() );

	deck_t table;

	deck_t* first_player  = &A;
	deck_t* battle_starter = 0;

	bool battle = false;
	int number_of_battles = 0;
	int battle_cards = 0;
	int turned_cards = 0;
	bool finite = true;

	while ( !(A.empty() || B.empty()) && finite ) {

		if ( finite ) {
			card_t c = put( table, *first_player );

			// update battle status
			switch ( c ) {
				case 1: 
				case 2:
				case 3:
					battle=true;
					number_of_battles++;
					battle_cards = c;
					battle_starter = first_player;
					break;
				default:
					if ( battle ) {
						battle_cards--;
						if ( battle_cards == 0 ) {
							battle = false;
							win( table, *battle_starter );
						}
					}
			}// end - update battle status

			// update first_player
			if ( battle ) {
				if ( first_player == battle_starter ) {
					myswap( &first_player, &A, &B );
				} else {
				}
			} else {
				myswap( &first_player, &A, &B );
			}// end - update first_player

			turned_cards++;
		}

		if ( turned_cards >= 475*35 ) {
			finite = false;
		}

	}

	std::string result;

	if ( A.empty() && B.empty() ) {
		result = "AB";
	} else if ( A.empty() ) {
		result = "B";
	} else if ( B.empty() ) {
		result = "A";
	} else if ( !finite ) {
		result = "infinite";
	}

	std::ostringstream res;
	res << result << "," << turned_cards << "," << number_of_battles 
		<< std::endl;

	return res.str();
}

int main(int argc, char* argv[] )
{
	int start_from = 0;
	if ( argc == 2 ) {
		std::istringstream iss(argv[1]);
		iss >> start_from;
	}

#ifdef COMPILE_ON_WINDOWS
	ULONGLONG begin = ptime();
#endif

	size_t size = 20;
	card_t battle_cards[]={1,2,3,
						   1,2,3,
						   /*1,2,3,
	                       1,2,3*/
						  };

	deck_t deck( size, 0 );
	size_t battle_cards_size = sizeof(battle_cards)/sizeof(card_t);
	deck_t::iterator sep = deck.begin() + battle_cards_size;
	for ( deck_t::iterator it = deck.begin(); it != sep; it++ ) {
		*it = battle_cards[ it - deck.begin() ];
	}
	bool verbose = false;
	for ( deck_t::iterator it = sep; verbose && it != deck.end(); it++ ) {
		*it = ++battle_cards_size;
	}

	std::sort( deck.begin(), deck.end() );

	int match = 0;
	std::cout << "match" << "," << "result" << "," << "turned_cards" << "," 
		      << "number_of_battles" << std::endl;

	do {
		if ( match >= start_from  ) {
			std::cout << match << "," << game( deck );
		}
		match++;
	} while ( std::next_permutation( deck.begin(), deck.end() ) );


#ifdef COMPILE_ON_WINDOWS
	ULONGLONG end = ptime();
	std::cerr << begin << "\t" << end << "\t" << end - begin << "\t" << (end-begin)*100e-9;
#endif

	return 0;
}

