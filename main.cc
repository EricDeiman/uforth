#include "main.h"

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

workStack& eval( instructions& insns, workStack& theStack, dictionary& theEnv ) {
  enum modes { evaluate, define };

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

  return theStack;
}

void print( workStack& theStack, ostream& output ) {
  for( auto s : theStack ) {
    output << s->str() << endl;
  }
}

void repl( int argc, char** argv ) {
  workStack theStack;
  dictionary theEnv;

  instructions primativeOps {
    make_shared< ufBinOp >( new ufAddOp() ),
      make_shared< ufBinOp >(  new ufSubOp() ),
      make_shared< ufBinOp >( new ufMulOp() ),
      make_shared< ufBinOp >( new ufDivOp() ),
      make_shared< ufBinOp >( new ufExpOp() ),
      make_shared< ufAssignOp >(),
      make_shared< ufBeginBlock >(),
      make_shared< ufMkBlock >(),
      make_shared< ufDupOp >(),
      make_shared< ufSwapOp >(),
      make_shared< ufPopOp >(),
      make_shared< ufTrueOp >(),
      make_shared< ufFalseOp >(),
      make_shared< ufIfOp >()
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
