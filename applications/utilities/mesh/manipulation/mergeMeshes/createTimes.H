    Info<< nl << "Create Times" << endl;

    const fileName masterCasePath = masterCase.path();
    const fileName masterCaseName = masterCase.name();

    Time runTimeMaster
    (
        Time::controlDictName,
        masterCasePath,
        masterCaseName
    );
    runTimeMaster.functionObjects().off();
    
	PtrList<Time> runTimesToAdd(addCases.size());
	forAll(addCases, i)
	{
		const fileName addCasePath = addCases[i].path();
		const fileName addCaseName = addCases[i].name();

		runTimesToAdd.set
		(
			i,
			new Time
			(
				Time::controlDictName,
				addCasePath,
				addCaseName
			)
		);
		runTimesToAdd[i].functionObjects().off();
	}
    /*
    const fileName addCasePath = addCase.path();
    const fileName addCaseName = addCase.name();

    Time runTimeToAdd
    (
        Time::controlDictName,
        addCasePath,
        addCaseName
    );
    runTimeToAdd.functionObjects().off();
    */
