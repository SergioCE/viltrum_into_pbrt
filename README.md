Compilation instructions: Cmake usual pipeline

Execution instructions:
    ./render_new <option> <sceneFile>

        - option: Integration option (Only 0 (MC) for render_new)
        - sceneFile: Path to PBRT scene (.pbrt file)

    ./render_old <option> <sceneFile>

        - option: Integration option:
            0: Monte Carlo
            1: Dyadic nets
            2: Control variates
        - sceneFile: Path to PBRT scene (.pbrt file)

Scenes:
    Uploaded a scenes folder, I've only tried it with redSphere and cornelBox, but it should work with any other scene of the folder.
    Use the path to the .pbrt file.