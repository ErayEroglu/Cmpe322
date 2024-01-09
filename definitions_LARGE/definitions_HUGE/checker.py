import subprocess

def compare_output(expected_output, actual_output):
    return expected_output == actual_output

def generate_unmatched_entry(filename, expected_output, actual_output):
    return {filename: {"expected": expected_output, "actual": actual_output}}

def main():
    count = 10000
    # Define the range of input files
    input_files = [f"def{i}" for i in range(1, count)]

    unmatched = []

    for input_file in input_files:
        # Copy the input file to definition.txt
        subprocess.run(["cp", f"{input_file}.txt", "definition.txt"])

        # Generate the command to execute the scheduler binary
        scheduler_command = "./scheduler"

        # Run the scheduler binary and capture the output
        actual_output = subprocess.check_output(scheduler_command, shell=True, text=True).splitlines()

        # Read the expected output from the file
        expected_output_file_name = f"out{input_file[3:]}.txt" if len(input_file) > 3 else f"out{input_file[-1]}.txt"
        with open(expected_output_file_name, "r") as expected_output_file:
            expected_output = expected_output_file.read().splitlines()

        # Compare and print the result
        if not compare_output(expected_output, actual_output):
            unmatched.append(generate_unmatched_entry(input_file, expected_output, actual_output))
    for entry in unmatched:
        print("Unmatched filename: " + list(entry.keys())[0])
        print("Expected Output:")
        print(entry[list(entry.keys())[0]]["expected"])
        print("Actual Output:")
        print(entry[list(entry.keys())[0]]["actual"])
        print()

if __name__ == "__main__":
    main()
