import argparse as ap


def parse_args() -> ap.Namespace:
    parser = ap.ArgumentParser()
    parser.add_argument("--result", type=str, help="Path to result file")
    parser.add_argument("--num-system", type=int, help="Number System")
    return parser.parse_args()


def main(*, result_file: str, num_system: int) -> None:
    no_errors = True

    with open(result_file, "r") as reader:
        f_line = reader.readline()
        n = int(f_line[f_line.find("=") + 1 : f_line.find(")")])

        for _ in range(n):
            r_str = reader.readline()
            a_str, b_str = reader.readline().split("*")

            r = int(r_str.replace("=", "").strip(), num_system)
            a = int(a_str.strip(), num_system)
            b = int(b_str.strip(), num_system)

            if a * b != r:
                no_errors = False
                print(f"{a} * {b} != {r}")

    print("All Passed!" if no_errors else "Some Failed!")


if __name__ == "__main__":
    args: ap.Namespace = parse_args()
    main(result_file=args.result, num_system=args.num_system)
