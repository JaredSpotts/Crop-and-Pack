import subprocess
import csv

CropAndPack_exe = r"C:\Users\jared\projects\slime-golf\CropAndPack\build\Release\CropAndPack.exe"
CropAndPack_root = r"C:\Users\jared\projects\slime-golf\CropAndPack"
output_file_name = "16_angle_benchmark_results.csv"

renders = [r"C:\Users\jared\projects\slime-golf\graphics\characters\KirbyLR\isometric\16A0.5\idle\render_info.json",
           r"C:\Users\jared\projects\slime-golf\graphics\characters\KirbyLR\isometric\16A1.0\idle\render_info.json",
           r"C:\Users\jared\projects\slime-golf\graphics\characters\KirbyLR\isometric\16A1.5\idle\render_info.json"
           ]

#thread_counts = ["1", "4", "8"]
thread_counts = ["1", "4", "8", "10", "12"]

output = [["Run number",
           "Mode",
           "Render scale",
           "Thread count", 
           "Start dimensions",
           "Crop dimensions",
           "Total time ms",
           "Crop/Pack time ms"]]

run_count = 8
def run_program(args, run_number, mode):
    result = subprocess.run(args,
                            cwd=CropAndPack_root,
                            text=True,
                            capture_output=True)
    if result.returncode != 0:
        raise RuntimeError(f"CropAndPack failed with code {result.returncode}")
    
    if run_number == 0:
        row = ["warmup"]
    else:
        row = [run_number]
    row.append(mode)
    
    lines = result.stdout.split("\n")
    start_index = lines.index("--BENCHMARK--")
    for i in range(start_index + 1, len(lines) - 1):
        row.append(lines[i])

    output.append(row)
    print(row)
    



for render in renders:
    for thread_count in thread_counts:
        args = [CropAndPack_exe, 
                    "--renders", 
                    render, 
                    "--threads", 
                    thread_count]
        for run_number in range(run_count):
            run_program(args, run_number, "fixed")
    
    # Dynamic threading
    args = [CropAndPack_exe, 
                "--renders", 
                render, 
                "--dynamic", 
                ]
    for run_number in range(run_count):
        run_program(args, run_number, "dynamic")
    

outfile = open(output_file_name, "w", newline="")
writer = csv.writer(outfile)
writer.writerows(output)
outfile.close()

# args = [CropAndPack_exe, 
#             "--renders", 
#             renders[0], 
#             "--threads", 
#             thread_counts[0]]
# run_program(args, 0)