/*
Pelagaletto
Alessandro Gentilini
Warning: the code is not polished.

GNU General Public License v3
$Date$
*/

#include <string>
#include <sstream>
#include <exception>

#include <algorithm>
#include <iostream>
#include <deque>

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

#include <iomanip>
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

#include <set>



#include <windows.h>

ULONGLONG ptime()
{
	FILETIME c, e, k, u;
	BOOL b = ::GetProcessTimes( ::GetCurrentProcess(), &c, &e, &k, &u );
	ULARGE_INTEGER li;
	li.HighPart = u.dwHighDateTime;
	li.LowPart = u.dwLowDateTime;
	return li.QuadPart;
}


int main(int argc, char* argv[])
{
	ULONGLONG begin = ptime();

	int count = 3;
	
	std::set< std::string > provate;

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
	do {
		/*
		std::string sdeck( stringify( deck ) );
		if ( provate.count( sdeck ) ) {
			continue;
		} else {
			provate.insert( sdeck );
		}
		*/

		deck_t A( deck.begin()       , deck.begin()+size/2 );
		deck_t B( deck.begin()+size/2, deck.end()          );

		/*
		std::cerr << "Start " << match << std::endl;
		std::cerr << "A " << stringify( A ) << std::endl;
		std::cerr << "B " << stringify( B ) << std::endl << std::endl;
		*/

		deck_t table;

		deck_t* first_player  = &A;
		deck_t* battle_starter;

		bool battle = false;
		int number_of_battles = 0;
		int battle_cards = 0;
		int turned_cards = 0;

		while ( !(A.empty() || B.empty()) ) {

			card_t c = put( table, *first_player );

			// aggiorna battle
			bool prev_battle = battle;
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
			}// fine - aggiorna battle

			// aggiorna first_player
			if ( battle ) {
				if ( first_player == battle_starter ) {
					myswap( &first_player, &A, &B );
				} else {
				}
			} else {
				myswap( &first_player, &A, &B );
			}// fine - aggiorna first_player
/*
			std::cerr << match << "." << turned_cards << std::endl;
			std::cerr << "A " << stringify( A ) << std::endl;
			std::cerr << "B " << stringify( B ) << std::endl;
			std::cerr << "T " << stringify( table ) << std::endl << std::endl;
*/
			turned_cards++;
		}

		std::string winner;

		if ( A.empty() && B.empty() ) {
			winner = "AB";
		}

		if ( A.empty() ) {
			winner = "B";
		}

		if ( B.empty() ) {
			winner = "A";
		}

		std::cout << match << "," << winner << "," << turned_cards << "," << number_of_battles << std::endl;

		match++;

	} while ( std::next_permutation( deck.begin(), deck.end() ) );


	ULONGLONG end = ptime();
	std::cout << begin << "\t" << end << "\t" << end - begin;

	return 0;
}

