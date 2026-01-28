with open("blacklist.txt", "r") as input_file, open("blocklist.txt", "w") as output_file:
    for line in input_file:
        parts = line.strip().split()
        print(parts)
        try:
            domain = parts[1]
            output_file.write(domain + "\n")
        except:
            continue

print("Done! Check blocklist.txt for your domains.")