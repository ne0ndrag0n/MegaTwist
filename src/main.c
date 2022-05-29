#include <genesis.h>
#include "bootstrap.h"

int main( u16 resetType ) {
	twist_bootstrap( resetType );

	while( 1 );
}