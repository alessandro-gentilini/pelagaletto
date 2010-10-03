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

class Romagnole
{
public:
	static const unsigned n_suits = 4;
	static const unsigned cards_per_suits = 10;
	static const unsigned min = 0;
	static const unsigned max = n_suits * cards_per_suits - 1;

	static std::string toString( int c ) throw ( std::exception )
	{
		if ( c < min || c > max ) throw std::exception("Unexpected card value!");
		std::ostringstream ss;
		ss << cards_desc[c%cards_per_suits] << " di " << suits_desc[c/cards_per_suits];
		return ss.str();
	}

	inline static int score( int c ){ return c%cards_per_suits; }

private:
	static const char* suits_desc[n_suits];
	static const char* cards_desc[cards_per_suits];
};

const char* Romagnole::suits_desc[Romagnole::n_suits] = {"Bastoni","Coppe","Denari","Spade"};
const char* Romagnole::cards_desc[Romagnole::cards_per_suits] = {"Asso","2","3","4","5","6","7","Fante","Cavallo","Re"};

#include <algorithm>
#include <iostream>
#include <deque>

typedef std::deque< int > deck_t;
typedef deck_t::value_type card_t;


void win( deck_t& table, deck_t& winner )
{
	while ( !table.empty() ) {
		winner.push_back( table.front() );
		table.pop_front();
	}
}

card_t put( deck_t& table, deck_t& putter )
{
	card_t f = putter.front();
	table.push_front( f );
	putter.pop_front();
	return f;
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

int main(int argc, char* argv[])
{
	for ( int p = 0; p< 1000000; p++ ) {
		deck_t deck;
		for ( int i = 0; i < Romagnole::n_suits * Romagnole::cards_per_suits; i++ ) {
			deck.push_back( i );
		}

		std::random_shuffle( deck.begin(), deck.end() );

		deck_t p1( deck.begin()   , deck.begin()+20 );
		deck_t p2( deck.begin()+20, deck.end()      );

		std::cout << stringify( p1 ) << std::endl;
		std::cout << stringify( p2 ) << std::endl;

		init( stringify( p1 ) );

		deck_t table;

		unsigned indice_calatore  = 1;//eliminarlo!
		unsigned indice_iniziatore_battle = 0;

		deck_t* calatore  = &p1;
		deck_t* iniziatore_battle;

		bool battle = false;
		int battle_cards = 0;
		int mosse = 0;

		while ( !(p1.empty() || p2.empty()) ) {
			
			switch( indice_calatore ) {
				case 1: calatore = &p1; break;
				case 2: calatore = &p2; break;
				default: throw std::exception("Unexpected number of players!");
			}

			switch( indice_iniziatore_battle ) {
				case 0: break;//nessuna battaglia in corso
				case 1: iniziatore_battle = &p1; break;
				case 2: iniziatore_battle = &p2; break;
				default: throw std::exception("Unexpected number of players!");
			}

			card_t c = put( table, *calatore );

			// aggiorna battle
			bool prev_battle = battle;
			switch ( c ) {
				case 0: 
					battle=true;
					battle_cards = 1;
					iniziatore_battle = calatore;
					indice_iniziatore_battle = indice_calatore;
					break;
				case 1: 
					battle=true;
					battle_cards = 2;
					iniziatore_battle = calatore;
					indice_iniziatore_battle = indice_calatore;
					break;
				case 2: 
					battle=true;
					battle_cards = 3;
					iniziatore_battle = calatore;
					indice_iniziatore_battle = indice_calatore;
					break;
				default:
					if ( battle ) {
						battle_cards--;
						if ( battle_cards == 0 ) {
							battle = false;
							win( table, *iniziatore_battle );
							indice_iniziatore_battle = 0;
						}
					}
			}// fine - aggiorna battle

			// aggiorna calatore
			if ( battle ) {
				if ( indice_calatore == indice_iniziatore_battle ) {
					switch( indice_calatore ) {//swap
						case 1: indice_calatore = 2; break;
						case 2: indice_calatore = 1; break;
						default: throw std::exception("Unexpected number of players!");
					}//fine swap
				} else {
				}
			} else {
				switch( indice_calatore ) {//swap
					case 1: indice_calatore = 2; break;
					case 2: indice_calatore = 1; break;
					default: throw std::exception("Unexpected number of players!");
				}//fine swap
			}// fine - aggiorna calatore

			mosse++;
		}

		if ( p1.empty() ) {
			std::cout << p << ",2," << mosse << std::endl;
		}

		if ( p2.empty() ) {
			std::cout << p << ",1," << mosse << std::endl;
		}
	}

	return 0;
}

