import random
import subprocess

def generate_file(file_number):
    filename = f"def{file_number}.txt"

    entries = []
    random_numbers = [random.randint(0, 10000) for i in range(10)]
    divisible_by_10 = [random.randint(0, 1000) * 10 for i in range(10)]
    random_numbers.extend(divisible_by_10)

    with open(filename, 'w') as file:
        process_zero = random.randint(1, 10)
        entry = f"P{process_zero} {random.randint(1, 10)} {0} "
        metal = random.choice(["GOLD", "SILVER", "PLATINUM"])
        entry += metal + "\n"
        entries.append(entry)

        for i in range(10):
            if (i + 1) == process_zero:
                continue
            entry = f"P{i+1} {random.randint(1, 10)} {random.choice(random_numbers)} "
            metal = random.choice(["GOLD", "SILVER", "PLATINUM"])
            entry += metal + "\n"
            entries.append(entry)
        random.shuffle(entries)
        file.write(''.join(entries))
    
    subprocess.run(["cp", filename, "definition.txt"])
    scheduler_command = "./scheduler"
    # Run the scheduler binary and capture the output
    actual_output = subprocess.check_output(scheduler_command, shell=True, text=True).splitlines()
    # Write the output to the file
    with open(f"out{file_number}.txt", "w") as output_file:
        output_file.write('\n'.join(actual_output))


if __name__ == "__main__":
    starting_index = 12
    count = 10000

    for i in range(starting_index, count + 1):
        generate_file(i)

    print(f"Files generated from def{starting_index}.txt to def{count}.txt")
