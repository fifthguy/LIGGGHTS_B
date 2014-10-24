/* ----------------------------------------------------------------------
   LIGGGHTS - LAMMPS Improved for General Granular and Granular Heat
   Transfer Simulations

   LIGGGHTS is part of the CFDEMproject
   www.liggghts.com | www.cfdem.com

   Christoph Kloss, christoph.kloss@cfdem.com
   Copyright 2009-2012 JKU Linz
   Copyright 2012-     DCS Computing GmbH, Linz

   LIGGGHTS is based on LAMMPS
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   http://lammps.sandia.gov, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   This software is distributed under the GNU General Public License.

   See the README file in the top-level directory.
------------------------------------------------------------------------- */

#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"
#include "style_command.h"
#include "universe.h"
#include "atom.h"
#include "atom_vec.h"
#include "comm.h"
#include "group.h"
#include "domain.h"
#include "output.h"
#include "thermo.h"
#include "force.h"
#include "pair.h"
#include "min.h"
#include "modify.h"
#include "compute.h"
#include "bond.h"
#include "angle.h"
#include "dihedral.h"
#include "improper.h"
#include "kspace.h"
#include "update.h"
#include "neighbor.h"
#include "special.h"
#include "variable.h"
#include "error.h"
#include "memory.h"
#include "input_multisphere.h"

using namespace LAMMPS_NS;

#define MAXLINE 2048
#define DELTA 4

InputMultisphere::InputMultisphere(LAMMPS *lmp, int argc, char **argv) : Input(lmp, argc, argv)
	{}

InputMultisphere::~InputMultisphere()
	{}

/* ----------------------------------------------------------------------
   process clump file
------------------------------------------------------------------------- */

int InputMultisphere::clmpfile(double **xclmp,double *rclmp,int nclmps)
	/******This function is checked for its return value inside next function******/
	{
		  int n;
		  int iClmp = 0;

		  while (1) 
				{
						// read one line from input script
						// if line ends in continuation char '&', concatenate next line(s)
						// n = str length of line
						if (me == 0) 
							{
								  if (fgets(line,MAXLINE,nonlammps_file) == NULL) n = 0;
								  else n = strlen(line) + 1;
								  while (n >= 3 && line[n-3] == '&')
										{
											if (fgets(&line[n-3],MAXLINE-n+3,nonlammps_file) == NULL) n = 0;
											else n = strlen(line) + 1;
										}
							}

						// bcast the line
						// if n = 0, end-of-file
						// error if label_active is set, since label wasn't encountered
						// if original input file, code is done
						// else go back to previous input file
						MPI_Bcast(&n,1,MPI_INT,0,world);
						
						if (n == 0)
							break;

						MPI_Bcast(line,n,MPI_CHAR,0,world);

						// if n = MAXLINE, line is too long
						if (n == MAXLINE) 
							{
								  char str[MAXLINE+32];
								  sprintf(str,"Input line too long: %s",line);
								  error->all(FLERR,str);
							}

						//parse one line from the clump file
						parse_nonlammps();

						//skip empty lines
						if(narg == 0)
							{
								if (me == 0) fprintf(screen,"Note: Skipping empty line or comment line in clump file\n");
								continue;
							}

						if(iClmp >= nclmps)
							error->all(FLERR,"Number of clumps in file larger than number specified");

						if(narg < 4)
							error->all(FLERR,"Not enough arguments in one line of clump file, need to specify [xcoo ycoo zcoo radius] in each line");


			/*****************Copying values from fragment file*********************/
						rclmp[iClmp] = atof(arg[3]);		//Copying radius values from fragment file//

						for(int j = 0; j < 3; j++)			//Copying position values from fragment file//
							xclmp[iClmp][j] = atof(arg[j]);
			/***********************************************************************/
						iClmp++;		//iclmp is the number of copying made for and should reach the number of fragment spheres//
				}

		return iClmp;
	}

/* ----------------------------------------------------------------------
   process all input from file
------------------------------------------------------------------------- */

void InputMultisphere::clmpfile(const char *filename, double **xclmp,double *rclmp,int nclmps)
	{
		  if (me == 0)
			  {
					nonlammps_file = fopen(filename,"r");		//Should be copying the contents of fragement file in nonlammps_file//
					if (nonlammps_file == NULL)			//of no value//
						{
							  char str[128];
							  sprintf(str,"Cannot open clump file %s",filename);
							  error->one(FLERR,str);
						}
			  }
		  else nonlammps_file = NULL;		//of no value//

		  if(clmpfile(xclmp,rclmp,nclmps) != nclmps)	//This is previous function defined in this file//
				error->all(FLERR,"Number of clumps in file does not match number of clumps that were specified");

		  if(nonlammps_file) fclose(nonlammps_file); 	//Writing is done in non-lammps file and it can be closed now//

	}
