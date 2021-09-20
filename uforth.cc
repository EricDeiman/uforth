/*
  The uforth programming language
  Copyright 2016 Eric J. Deiman
  This file is part of the uforth programming language.
  The uforth programming language is free software: you can redistribute it
  and/ormodify it under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your option) any
  later version.
  The uforth programming language is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
  You should have received a copy of the GNU General Public License along with the
  uforth programming language. If not, see <https://www.gnu.org/licenses/>
*/

#include "uforth.hh"

int getInteger( string in ) {
  stringstream ss( in );
  int n;

  ss >> n;

  return n;
}

instructions& read( string prompt, ostream& output, istream& input ) {
  instructions* is = new instructions();
  instructions& insns = *is;

  stringbuf buffer;

  if( prompt.size() ) {
    output << prompt;
  }
  input.get( buffer );
  input.ignore( 1 ); // eat the \n

  stringstream ss( buffer.str() );

  while( ss ) {
    stringbuf word;
    ss.get( word, ' ' );
    if( !ss.eof() ) {
      ss.ignore( 1 ); // eat the delimiter
    }

    string in = word.str();

    if( '0' <= in.front() && in.front() <= '9' ) {
      insns.push_back( make_shared< ufInteger >( getInteger( in ) ) );
    }
    else if( in.size() > 0 ) {
      insns.push_back( make_shared< ufSymbol >( in ) );
    }
  }

  return insns;
}

workStack& eval( instructions& insns, workStack& theStack, environment& theEnv ) {
  modes mode = evaluate;
  int blockDepth = 0;

  for( auto i : insns ) {
    if( i->str() == blockBegin ) {
      blockDepth++;
    }
    else if( i->str() == blockEnd ) {
      blockDepth--;
    }

    mode = blockDepth ? define : evaluate;

    switch( mode ) {
    case evaluate:
      i->eval( theStack, theEnv );
      break;
    case define:
      theStack.push_front( i );
      break;
    }
  }

  delete &insns;
  return theStack;
}

void print( workStack& theStack, ostream& output ) {
  for( auto s : theStack ) {
    output << s->str() << endl;
  }
}

void repl( int argc, char** argv ) {
  workStack theStack;
  environment theEnv( NULL );

  instructions primativeOps {
    make_shared< ufBinOp< ufAddOp > >(),
      make_shared< ufBinOp< ufSubOp > >(),
      make_shared< ufBinOp< ufMulOp > >(),
      make_shared< ufBinOp< ufDivOp > >(),
      make_shared< ufBinOp< ufExpOp > >(),
      make_shared< ufBinOp< ufModOp > >(),
      make_shared< ufNegOp >(),
      make_shared< ufAssignOp >(),
      make_shared< ufBeginBlock >(),
      make_shared< ufMkBlock >(),
      make_shared< ufDupOp >(),
      make_shared< ufSwapOp >(),
      make_shared< ufPopOp >(),
      make_shared< ufBinOp< ufLtOp > >(),
      make_shared< ufBinOp< ufLeOp > >(),
      make_shared< ufBinOp< ufGtOp > >(),
      make_shared< ufBinOp< ufGeOp > >(),
      make_shared< ufBinOp< ufEqOp > >(),
      make_shared< ufBinOp< ufNeOp > >(),
      make_shared< ufIfOp >(),
      make_shared< ufBooleanOp >( true ),
      make_shared< ufBooleanOp >( false ),
      make_shared< ufLoopOp >()
      };

  for( auto o : primativeOps ) {
    theEnv[ o->str() ] = o;
  }

  if( argc == 2 ) {
    stringstream ss( argv[ 1 ] );
    print( eval( read( "", cout, ss ), theStack, theEnv ), cout );
  }
  else {
    while( true ) {
      print( eval( read( "prompt> ", cout, cin ), theStack, theEnv ), cout );
    }
  }
}

int main( int argc, char** argv ) {

  repl( argc, argv );

  return 0;
}
