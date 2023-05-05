# usage:
# -t tracefile
# -c correctness (will include correctness testing)
# -f number of faulty nodes (total nodes = 3f + 1)
# -b Byzantine mode of service
# for PBFT, -b 0 will run only good nodes, -b 1 will run non responsive nodes
# -b 2 will run wrong but not non responsive nodes
# -m mode, -m pbft will run pbft, -m bbalgo will run our algorithm, -m nrep will run not-replicated service only
# This program will run non-replicated service and your own service, and compare the
# performance of it on the tracefile

import argparse
import os
import subprocess
import time

# Run program, return string with results
def run_prog(tracefile, f, byzantine_mode, mode, delta=0):
  if f <= 0:
    print("f is " + str(f) + ", which is not a valid value of f. f should be > 0.")
    exit()
  
  if (byzantine_mode < 0 or byzantine_mode > 2):
    print("b is " + str(byzantine_mode) + ", which is not a valid value of b. b should be 0, 1, or 2.")
    exit()

  if (mode < 0 or mode > 2):
    print("mode is " + str(mode) + ", which is not a valid value of of mode. mode should be either 0, 1, 2.")
    exit()

  # Check to make sure you can open tracefile
  absolute_path = os.path.dirname(__file__)
  relative_file_path = "../traces"
  trace_path = os.path.join(absolute_path, relative_file_path)
  file_path = os.path.join(trace_path, tracefile)

  if (not os.path.exists(file_path) or not os.path.isfile(file_path)):
    print("tracefile " + str(tracefile) + " does not exist at path " + str(file_path))
    exit()

  absolute_path = os.path.dirname(__file__)
  relative_path = "../build"
  executable_dir_path = os.path.join(absolute_path, relative_path)
  exec_relative_path = "bft-sim"
  executable_path = os.path.join(executable_dir_path, exec_relative_path)
  if (os.path.exists(executable_path)):
    # Run command if all arguments pass validation
    # This assumes that ./build/bft-sim exists
    print(executable_path)
    tracefile_path = "../traces/" + tracefile
    bft_sim_cmd = "./bft-sim -f " + str(f) + " -t " + tracefile_path + " -b " + str(byzantine_mode) + " -r true -m " + str(mode) + " -d " + str(delta)
    print(bft_sim_cmd)
    start_time = time.time()
    process1 = subprocess.run(bft_sim_cmd, shell=True, capture_output=True, text=True, cwd=executable_dir_path)
    end_time = time.time()
    return process1.stdout, end_time - start_time
  else:
    print("bft-sim doesn't exist in CURR_WORKING_DIR/build/bft-sim")
    exit()

def error_msg(nrep_list, add_list, mode):
  print("Correctness check failed.")
  print("Non replicated service results are:")
  print(nrep_list)
  if mode == "pbft":
    print("PBFT results are:")
  else:
    print("BBAlgo results are:")
  print(add_list)

if __name__ == "__main__":
  parser = argparse.ArgumentParser(
                    description='runs testing and benchmarking for programs')
  parser.add_argument('-t', '--tracefile', action='store', help='tracefile that we are running benchmarking on')
  parser.add_argument('-c', '--correctness', action='store_true', help='whether we run correctness tests on the simulator')
  parser.add_argument('-f', '--faulty-nodes', action='store', help='number of faulty nodes (total nodes = 3f + 1)', type=int) 
  parser.add_argument('-b', '--byzantine-mode', action='store', help='byzantine mode: for PBFT, -b 0 will run only good nodes, -b 1 will run non responsive nodes, -b 2 will run wrong but not non responsive nodes', type=int)
  parser.add_argument('-m', '--mode', action='store', help='-m pbft will run pbft, -m bbalgo will run our algorithm, -m nrep will run not-replicated service only')
  parser.add_argument('-d', '--delta', action='store', help='delta, for BBAlgo')
  args = parser.parse_args()
  args_dict = vars(args)
  print(args_dict)

  tracefile = args_dict['tracefile']
  f = args_dict['faulty_nodes']
  byzantine_mode = args_dict['byzantine_mode']
  prog_mode = args_dict['mode']
  delta = args_dict['delta']

  non_replicated_results, nrep_time = run_prog(tracefile, f, byzantine_mode, 0)
  print("Non replicated service took " + str(round(nrep_time, 4)) + " seconds to run on tracefile " + tracefile)
  additional_results = ''

  if prog_mode == "pbft":
    pbft_results, pbft_time = run_prog(tracefile, f, byzantine_mode, 1)
    print("PBFT service took " + str(round(pbft_time, 4)) + " seconds to run on tracefile " + tracefile)
    additional_results = pbft_results
  elif prog_mode == "bbalgo":
    bbalgo_results, bbalgo_time = run_prog(tracefile, f, byzantine_mode, 2, delta)
    print("BBAlgo service took " + str(round(bbalgo_time, 4)) + " seconds to run on tracefile " + tracefile)
    additional_results = bbalgo_results

  if args_dict['correctness'] and prog_mode != "nrep":
    print("Starting correctness check...")
    nrep_list = non_replicated_results.splitlines()
    add_list = additional_results.splitlines()

    if len(nrep_list) != len(add_list):
      error_msg(nrep_list, add_list, prog_mode)
      exit()
    
    for i in range(0, len(nrep_list)):
      if nrep_list[i] != add_list[i]:
        error_msg(nrep_list, add_list, prog_mode)
        exit()
    
    print("Correctness check for non replicated service and " + prog_mode + " service succeeded.")
