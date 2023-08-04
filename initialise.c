#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h> 
#include "stream_post_ioserver.h"

void initialise(int argc, char** argv, struct params *ioParams)
{
	while (1)
	{
		static struct option long_options[] =
		{
			{"N",  required_argument, 0, 'a'}, 
			{"io",  required_argument, 0, 'b'}, 
			{0, 0}
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		long int c = getopt_long (argc, argv, "a:b:",						long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
			case 'a':
				ioParams->N = atoi(optarg); 

			case 'b':
				ioParams->ioLibNum = atoi(optarg); 

			case '?':
				/* getopt_long already printed an error message. */
				break;

			default:
				abort ();
		}
	}
} 

