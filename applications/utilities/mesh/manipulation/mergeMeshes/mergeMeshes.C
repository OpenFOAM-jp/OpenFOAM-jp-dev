/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2018 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Application
    mergeMeshes

Description
    Merges some meshes.

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "Time.H"
#include "mergePolyMesh.H"

using namespace Foam;

void getRootCase(fileName& casePath)
{
    casePath.clean();

    if (casePath.empty() || casePath == ".")
    {
        // handle degenerate form and '.'
        casePath = cwd();
    }
    else if (casePath[0] != '/' && casePath.name() == "..")
    {
        // avoid relative cases ending in '..' - makes for very ugly names
        casePath = cwd()/casePath;
        casePath.clean();
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Merges some meshes.\nSecond 'addCase' can be ignored."
    );

    argList::noParallel();
    #include "addOverwriteOption.H"

    argList::validArgs.append("masterCase");
    argList::addOption
    (
        "masterRegion",
        "name",
        "specify alternative mesh region for the master mesh"
    );

    argList::validArgs.append("addCase");
    argList::addOption
    (
        "addRegion",
        "name",
        "specify alternative mesh region for the additional mesh"
    );
    argList::addOption
    (
        "addCases",
        "names",
        "Specify cases to merge : -addCases '(caseA caseB)'"
    );
    argList::addOption
    (
        "addRegions",
        "names",
        "Specify alternative mesh regions for the additional meshes : -addRegions '(regionA regionB)'"
    );

    argList args(argc, argv, false);
    if (!args.check())
    {
         if(args.size()<2 && (! args.validOptions.found("addCases") || ! args.validOptions.found("addRegions")))
         {
			 FatalError.exit();
		 }
		 else
		 {
			 Info << "args 'addCase' is now optional. '-addCases' or 'addRegions' is used iinstead." << endl;
		 }
    }

    const bool overwrite = args.optionFound("overwrite");

    fileName masterCase = args[1];
    word masterRegion = polyMesh::defaultRegion;
    args.optionReadIfPresent("masterRegion", masterRegion);

    getRootCase(masterCase);
    
    // fileName addCase = args[2];
    word addRegion = polyMesh::defaultRegion;
    args.optionReadIfPresent("addRegion", addRegion);

    wordList addRegions(0);
    args.optionReadIfPresent("addRegions", addRegions);
    wordList tmpAddCases(0);
    args.optionReadIfPresent("addCases", tmpAddCases);
        
    List<fileName> addCases;
    if(args.size() > 2) 
    {
		addCases.append(args[2]);
		addRegions.resize(addRegions.size()+1);
		for(int i=addRegions.size()-1;i>0;i--)
		{
			addRegions[i] = addRegions[i-1];
		}
		addRegions[0] = addRegion;
	}
    else
    {		
		// mergeMeshed masterCase -addCases '(caseA caseB)' -addRegions '(region_a region_b)'
        if(tmpAddCases.size() == addRegions.size())
        {
            forAll(tmpAddCases, i)
            {
                addCases.append(fileName(tmpAddCases[i]));
            }
        }
		// mergeMeshed masterCase -addRegions '(region_a region_b)'
        else if(tmpAddCases.size() == 0)
        {
            forAll(addRegions, i)
            {
                addCases.append(fileName("."));
            }
        }
		// mergeMeshed masterCase -addCases '(caseA caseB)'
        else if(addRegions.size() == 0)
        {
            forAll(tmpAddCases, i)
            {
                addCases.append(fileName(tmpAddCases[i]));
                addRegions.append(polyMesh::defaultRegion);
            }
        }
    }
        
    //getRootCase(addCase);
    forAll(addCases, i)
    {
        getRootCase(addCases[i]);
    }
    
    Info<< "Master:      " << masterCase << "  region " << masterRegion << endl;
    //    << "mesh to add: " << addCase    << "  region " << addRegion << endl;
    forAll(addCases, i)
    {
         Info << "mesh to add: " << addCases[i] << "  region " << addRegions[i] << endl;
    }

    #include "createTimes.H"

    Info<< "Reading master mesh for time = " << runTimeMaster.timeName() << nl;

    Info<< "Create mesh\n" << endl;
    mergePolyMesh masterMesh
    (
        IOobject
        (
            masterRegion,
            runTimeMaster.timeName(),
            runTimeMaster
        )
    );
    const word oldInstance = masterMesh.pointsInstance();


    //Info<< "Reading mesh to add for time = " << runTimeToAdd.timeName() << nl;

    // Info<< "Create mesh\n" << endl;
    PtrList<polyMesh> meshesToAdd(addCases.size());
    forAll(addCases, i)
    {
        Info<< "Reading mesh to add for time = " << runTimesToAdd[i].timeName() << nl;
        Info<< "Create mesh\n" << endl;
        meshesToAdd.set
        (
            i,
            new polyMesh
            (
                IOobject
                (
                    addRegions[i],
                    runTimesToAdd[i].timeName(),
                    runTimesToAdd[i]
                )
            )
        );
    }
    /*
    polyMesh meshToAdd
    (
        IOobject
        (
            addRegion,
            runTimeToAdd.timeName(),
            runTimeToAdd
        )
    );
    */

    // masterMesh.addMesh(meshToAdd);
    // masterMesh.merge();
    forAll(addCases, i)
    {
		if (!overwrite)
		{
			runTimeMaster++;
			Info<< "Writing combined mesh to " << runTimeMaster.timeName() << endl;
		}
        masterMesh.addMesh(meshesToAdd[i]);
        masterMesh.merge();
    }

    if (overwrite)
    {
        masterMesh.setInstance(oldInstance);
    }

    masterMesh.write();

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
