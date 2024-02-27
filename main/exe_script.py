import subprocess
import time
import os


#Total number of samples
spps = [1024]
#Monte carlo samples
mcs = [16]
#Factor used to calculate spp_cv
cvs = [16]
"""
Integration technique:
    0: Control variates
    1: Variance reduction alpha optimized
    2: Importance sampling
    3: Control variates (alpha=1 and uniform region)
"""
var_red = [0,1,2,3,4,5,6,7,8]

executable_path = "../bin"

#Path to the results folder
results_root = "results"
os.makedirs(results_root, exist_ok=True)

scene_root = "../scenes/pbrt-v4-scenes"

#[f"{scene_root}/sanmiguel/sanmiguel-courtyard-second.pbrt","sanmiguel_second"],[f"{scene_root}/barcelona-pavilion/pavilion-day.pbrt","barcelona"],[f"{scene_root}/sanmiguel/sanmiguel-courtyard.pbrt","sanmiguel"],["../scenes/sssdragon/dragon_10.pbrt","dragon"], ["../scenes/redSphere/redSphere.pbrt","redSphere"]
#["../scenes/redSphere/redSphere.pbrt","redSphere4"]
scenes = [[f"{scene_root}/barcelona-pavilion/pavilion-day.pbrt","barcelona"]]

for scene in scenes:
    for var in var_red:
        for spp in spps:
            for mc in mcs:
                for cv in cvs:
                    if(var==0):
                        tec = "rr_uniform_alpha_opt"
                    elif(var==1):
                        tec = "rr_uniform_alpha0"
                    elif(var==2):
                        tec = "rr_uniform_alpha1"  
                    elif(var==3):
                        tec = "rr_integral_alpha_opt"
                    elif(var==4):
                        tec = "rr_integral_alpha0"
                    elif(var==5):
                        tec = "rr_integral_alpha1"   
                    elif(var==6):
                        tec = "rr_error_alpha_opt"
                    elif(var==7):
                        tec = "rr_error_alpha0"
                    elif(var==8):
                        tec = "rr_error_alpha1" 
                    output = f"{results_root}/{scene[1]}/{scene[1]}_{tec}_4dim_{cv}cv_{mc}mc_{spp}spp.hdr"
                    os.makedirs(f"{results_root}/{scene[1]}", exist_ok=True)

                    command = f"{executable_path}/render_control_variates -f {scene[0]} -cv {cv} -mc {mc} -v {var} -s {spp} -o {output}"
                    start_time = time.time()
                    try:
                        # Execute the command using subprocess
                        subprocess.run(command, shell=True, check=True)
                        print("Program executed successfully")
                    except subprocess.CalledProcessError as e:
                        print(f"Error executing program: {e}")
                    end_time = time.time()
                    elapsed_time = end_time - start_time
                    # Open the file in write mode
                    with open(f"{results_root}/{scene[1]}/times.txt", 'a') as file:
                        # Write the content to the file
                        file.write(f"{output}:\t\t{elapsed_time}\n")