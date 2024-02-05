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

    ./render_monte_carlo <flags>
        Flags:
            -f: Path to PBRT scene (.pbrt file)
            -s: Samples per pixel

    ./render_newton_cotes <flags>
        Flags:
            -f: Path to PBRT scene (.pbrt file)

    ./render_adaptive <flags>
        Flags:
            -f: Path to PBRT scene (.pbrt file)
            -i: Adaptive iterations

    ./render_control_variates <flags>
        Flags:
            -f: Path to PBRT scene (.pbrt file)
            -i: Adaptive iterations
            -s: Samples per pixel

Scenes:
    Uploaded a scenes folder, I've only tried it with redSphere and cornelBox, but it should work with any other scene of the folder.
    I uploaded a scenes zip to google drive: https://drive.google.com/file/d/135p-uCG-NGDXegVbHDrJO_sdtvY3Sry3/view?usp=sharing