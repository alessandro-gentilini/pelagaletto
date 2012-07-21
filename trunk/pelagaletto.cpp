//
// Pelagaletto
// Pelagaletto is an Italian card game similar to Beggar-My-Neighbour. 
//
// Warning: the code is not polished!
//
// Author : Alessandro Gentilini
// Source : http://code.google.com/p/pelagaletto/
// License: GNU General Public License v3
//
// $Rev$
// $Date$
//

#include <deque>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <bitset>

#include <unordered_map>

#include <ctime>

typedef std::deque<int> CardDeck;
typedef size_t GameID;

// Convert the card deck d to a string
std::string to_string( const CardDeck& d )
{
   std::ostringstream oss;
   const size_t sz = d.size();
   for( size_t i = 0; i < sz; i++ ) {
      oss << d[i];
   }
   return oss.str();
}

// Convert a string to a card deck
CardDeck init( const std::string& s )
{
   CardDeck r;
   for ( size_t i = 0; i < s.length(); i++ ) {
      switch( s[i] ) {
      case '1': 
      case 'J': 
         r.push_back(1); break;
      case '2': 
      case 'Q': 
         r.push_back(2); break;
      case '3':
      case 'K':
         r.push_back(3); break;
      case 'A': r.push_back(4); break;
      default: r.push_back(0); break;
      }
   }
   return r;
}

// I will keep the addresses of the two player's card decks
CardDeck* address_A;
CardDeck* address_B;

// Return the char identifier of the card deck whose address is p
char whois( CardDeck* p )
{
   if ( p == address_A )
      return 'A';
   else if ( p == address_B )
      return 'B';
   else if ( p == 0 )
      return 'N';
   else
      throw "address changed?";
}

// Operator Less for a bitset of N bits.
template <size_t N>
class Less 
{ 
public:
   bool operator() (const std::bitset<N> &lhs, 
      const std::bitset<N> &rhs) const 
   { 
      size_t i = N;
      while ( i > 0 ) {
         if ( lhs[i-1] == rhs[i-1] ) {
            i--;
         } else if ( lhs[i-1] < rhs[i-1] ) {
            return true;
         } else {
            return false;
         }
      }
      return false;
   } 
}; 

// Hash operator for a bitset of N bits.
// It is adapted from the functor std::tr1::hash<std::string> that comes from 
// the header <xfunctional> shipped with Microsoft Visual Studio 2010 Express
template <size_t N>
class Hash 
{ 
public:
	size_t operator() (const std::bitset<N> &bs) const 
		{	// hash _Keyval to size_t value by pseudorandomizing transform
		size_t _Val = 2166136261U;
		size_t _First = 0;
		size_t _Last = N;
		size_t _Stride = 1 + _Last / 10;

		for(; _First < _Last; _First += _Stride)
			_Val = 16777619U * _Val ^ (size_t)bs[_First];
		return (_Val);
		}   
}; 

// The status of a game is composed by:
// -battle flag
// -pay (how many cards one has to pay)
// -player id who started the battle (if any)
// -first player id
// -second player id
// -card deck of player A
// -card deck of player B
// -card deck on the table

const size_t N_CARDS         = 20;
const size_t MAX_BATTLE_CARD = 3;
const size_t BITS_PER_CODE   = 3;

const size_t CODE_NONE = MAX_BATTLE_CARD + 1;
const size_t CODE_A    = CODE_NONE + 1;
const size_t CODE_B    = CODE_A + 1;
const size_t CODE_SEP  = CODE_B + 1;

const size_t N_BATTLE_FLAG = 1;//battle
const size_t N_PAY         = 1;//pay
const size_t N_ROLES       = 3;//battle_starter,starter,answerer
const size_t N_SEPS        = 3;//A,B,table

const size_t BIT_STATUS_SIZE = BITS_PER_CODE*(N_BATTLE_FLAG+N_PAY+N_ROLES+N_SEPS) 
                               + BITS_PER_CODE*N_CARDS;

// The status of a game is stored as a bitset.
typedef std::bitset<BIT_STATUS_SIZE> BitStatus;

// A test for keeping the statuses of the games in an (ordered) map
//typedef std::map< BitStatus, GameID, Less< BIT_STATUS_SIZE > > StatusBitMap;

// The unordered map of the statuses of the games
typedef std::unordered_map< BitStatus, GameID, Hash< BIT_STATUS_SIZE > > StatusBitMap;

// A map where the status is stored as a string (it needs more memory than the bitset)
typedef std::map< std::string, GameID > StatusStringMap;

bool keep_info_between_games = false;

// Return the bitset identifier of the card deck whose address is p
BitStatus whois2( CardDeck* p )
{
   if ( p == 0 )
      return BitStatus(static_cast<unsigned long long>(CODE_NONE));
   else if ( p == address_A )
      return BitStatus(static_cast<unsigned long long>(CODE_A));
   else if ( p == address_B )
      return BitStatus(static_cast<unsigned long long>(CODE_B));
   else
      throw std::exception("address changed?");
}

// Convert the deck d to a biset representation
BitStatus to_string2( const CardDeck& d )
{
   BitStatus r;
   for ( size_t i = 0; i < d.size(); i++ ) {
      r <<= BITS_PER_CODE;
      r  |= d[i];
   }
   return r;
}

// Play one game, it uses an unordered map of bitset games statuses
// in order to detect an infinite loop game.
void play_game(
   const CardDeck& deck,
   CardDeck& A,
   CardDeck& B,
   CardDeck& table,
   CardDeck* starter,
   CardDeck* answerer,
   CardDeck* battle_starter,
   StatusBitMap& statuses,
   GameID& cnt,
   size_t& n_cards,
   size_t& n_battles,
   bool check_for_infinite_loop = true
   )
{
   int pay   = 0;
   size_t n  = 0; 
   size_t nb = 0;

   bool stop  = false;
   bool battle = false;
   while ( !stop && !A.empty() && !B.empty() ) {
      if ( check_for_infinite_loop ) {
         BitStatus bs;
         bs.set();
         bs <<= BITS_PER_CODE;
         bs  |= BitStatus(battle);
         bs <<= BITS_PER_CODE;
         bs  |= BitStatus(static_cast<unsigned long long>(pay));
         bs <<= BITS_PER_CODE;
         bs  |= whois2(battle_starter);
         bs <<= BITS_PER_CODE;
         bs  |= whois2(starter);
         bs <<= BITS_PER_CODE;
         bs  |= whois2(answerer);
         bs <<= BITS_PER_CODE;
         bs  |= BitStatus(static_cast<unsigned long long>(CODE_SEP));
         bs <<= (BITS_PER_CODE*A.size());
         bs  |= to_string2(A);
         bs <<= BITS_PER_CODE;
         bs  |= BitStatus(static_cast<unsigned long long>(CODE_SEP));
         bs <<= (BITS_PER_CODE*B.size());
         bs  |= to_string2(B);
         bs <<= BITS_PER_CODE;
         bs  |= BitStatus(static_cast<unsigned long long>(CODE_SEP));
         bs <<= (BITS_PER_CODE*table.size());
         bs  |= to_string2(table); 

         //std::cout << bs << "\n";

         //std::ostringstream oss;
         //oss << "A" << to_string(A) << "B" << to_string(B) << "T" << to_string(table) 
         //    << whois(starter) << whois(answerer) << battle << whois(battle_starter) 
         //    << pay;
         //std::string status = oss.str();
         //std::cout << status << "\n";

         if ( statuses.count( bs ) ) {
            if ( statuses[bs] == cnt ) {
               std::cout << cnt << " infinite " << to_string(deck) << "\n";
            } else {
               std::cout << cnt << "->" << statuses[bs] << "\n";
            }
            stop = true;
         } else {
            statuses.insert( std::make_pair( bs, cnt ) );
         }
      }

      if ( !stop ) {
         //std::cout<< "A:" << to_string(A) << "\n";
         //std::cout<< "B:" << to_string(B) << "\n";
         //std::cout<< "T:" << to_string(table) << "\n\n";

         table.push_front( starter->front() );
         starter->pop_front();
         n++;
         if ( 0 != table.front() ) {
            battle = true;
            battle_starter = starter;
            pay = table.front();
            std::swap( starter, answerer );
         } else {
            if ( battle ) {
               pay--;
               if ( pay==0 ) {
                  battle = false;
                  std::reverse( table.begin(), table.end() );
                  std::copy( table.begin(), table.end(), 
                     std::back_inserter(*battle_starter) );
                  table.clear();
                  nb++;

                  //std::cout<< "A:" << to_string(A) << "\n";
                  //std::cout<< "B:" << to_string(B) << "\n";
                  //std::cout<< "T:" << to_string(table) << "\n\n";

                  std::swap( starter, answerer );
               }
            } else {
               std::swap( starter, answerer );
            }
         }
      }
   } 
   table.clear();
   n_cards   = n;
   n_battles = nb;
}

// Play one game, it uses an ordered map of string games statuses
// in order to detect an infinite loop game.
void play_game_1( CardDeck& A,
   CardDeck& B,
   CardDeck& table,
   CardDeck* starter,
   CardDeck* answerer,
   CardDeck* battle_starter,
   StatusStringMap& statuses,
   GameID& cnt
   )
{
   bool battle = false;
   int pay = 0;
   size_t n = 0;
   bool stop = false;
   while ( !stop && !A.empty() && !B.empty() ) {

      std::ostringstream oss;
      oss << "A" << to_string(A) 
         << "B" << to_string(B) 
         << "T" << to_string(table) 
         << whois(starter) 
         << whois(answerer) << battle << whois(battle_starter) << pay;
      std::string status = oss.str();
      if ( statuses.count( status ) ) {
         if ( statuses[status]==cnt ) {
            std::cout << "infinite:" << cnt << "\n";
         } else {
            std::cout << cnt << "->" << statuses[status] << "\n";
         }
         stop = true;
      } else {
         statuses.insert( std::make_pair( status, cnt ) );
      }

      if ( !stop ) {

         std::cout<< "A:" << to_string(A) << "\n";
         std::cout<< "B:" << to_string(B) << "\n";
         std::cout<< "T:" << to_string(table) << "\n\n";

         table.push_front( starter->front() );
         starter->pop_front();
         n++;
         if ( 0 != table.front() ) {
            battle = true;
            battle_starter = starter;
            pay = table.front();
            std::swap( starter, answerer );
         } else {
            if ( battle ) {
               pay--;
               if ( pay==0 ) {
                  battle = false;
                  std::reverse( table.begin(), table.end() );
                  std::copy( table.begin(), table.end(), 
                     std::back_inserter(*battle_starter) );
                  table.clear();

                  std::cout<< "A:" << to_string(A) << "\n";
                  std::cout<< "B:" << to_string(B) << "\n";
                  std::cout<< "T:" << to_string(table) << "\n\n";

                  std::swap( starter, answerer );
               }
            } else {
               std::swap( starter, answerer );
            }
         }
      }
   } 

   std::cout << cnt << "," << n << "\n";
   table.clear();
   statuses.clear();
   cnt++;
}

// A test with some public internet available records.
bool test()
{
   CardDeck A;
   CardDeck B;

   StatusBitMap statuses;
   CardDeck table;

   address_A = &A;
   address_B = &B;

   CardDeck* starter  = address_A;
   CardDeck* answerer = address_B;
   CardDeck* battle_starter = 0;

   GameID cnt = 0;
   size_t n_cards   = 0;
   size_t n_battles = 0;

   bool passed = true;

   // http://www2.math.uu.se/~rmann/Miscellaneous.html
   char Mann_A[] = "K-KK----K-A-----JAA--Q--J-";
   char Mann_B[] = "---Q---Q-J-----J------AQ--";
   A = init(Mann_A);
   B = init(Mann_B);
   play_game(CardDeck(),A,B,table,starter,answerer,battle_starter,statuses,cnt,
      n_cards,n_battles,false);
   if ( n_cards != 7157 || n_battles != 1007 ) {
      std::cerr << "Failed test: Mann\n";
      passed = false;
   }

   // http://www2.math.uu.se/~rmann/Miscellaneous.html
   // http://people.virginia.edu/~rcn9f/
   char Nessler_1_A[] = "----Q------A--K--A-A--QJK-";
   char Nessler_1_B[] = "-Q--J--J---QK---K----JA---";
   A = init(Nessler_1_A);
   B = init(Nessler_1_B);
   play_game(CardDeck(),A,B,table,starter,answerer,battle_starter,statuses,cnt,
      n_cards,n_battles,false);
   if ( n_cards != 7207 || n_battles != 1015 ) {
      std::cerr << "Failed test: Nessler 1\n";
      passed = false;
   }

   // http://www2.math.uu.se/~rmann/Miscellaneous.html
   // http://people.virginia.edu/~rcn9f/   
   char Nessler_2_A[] = "-J-------Q------A--A--QKK-";
   char Nessler_2_B[] = "-A-Q--J--J---Q--AJ-K---K--";
   A = init(Nessler_2_A);
   B = init(Nessler_2_B);
   play_game(CardDeck(),A,B,table,starter,answerer,battle_starter,statuses,cnt,n_cards,
      n_battles,false);
   if ( n_cards != 7224 || n_battles != 1016 ) {
      std::cerr << "Failed test: Nessler 2\n";
      passed = false;
   }

   return passed;
}

int main() 
{    
   if (!test()) return 1;

   CardDeck deck = init("12300000001230000000");
   if ( deck.size() != N_CARDS ) {
      std::cerr << "error: deck size and status size mismatch\n";
      return 2;
   }
   std::sort(deck.begin(),deck.end());
   CardDeck A;
   CardDeck B;

   StatusBitMap statuses;
   CardDeck table;

   address_A = &A;
   address_B = &B;

   CardDeck* starter  = address_A;
   CardDeck* answerer = address_B;
   CardDeck* battle_starter = 0;

   GameID cnt = 0;
   size_t n_cards   = 0;
   size_t n_battles = 0;

   clock_t before;
   double elapsed;
   before = clock();
   
   do{
      A = CardDeck(deck.begin()              , deck.begin()+deck.size()/2);
      B = CardDeck(deck.begin()+deck.size()/2, deck.end()                );
            
      try{
         play_game(deck,A,B,table,starter,answerer,battle_starter,statuses,cnt,n_cards,n_battles);
         if ( !keep_info_between_games ) {
            statuses.clear();
         }
         std::cout << cnt << "," << n_cards << "\n";
         cnt++;
      }catch( const std::exception& e ) {
         std::cerr << "error: " << e.what() << "\n";
      }catch(...){
         std::cerr << "unknown error\n";
      }
      if ( cnt % 10000 == 0 ) {
         elapsed = clock() - before;
         std::cerr << elapsed/CLOCKS_PER_SEC << "," << statuses.size() << "\n";
      }
   }while( std::next_permutation( deck.begin(), deck.end() ) );

   return 0; 
}
