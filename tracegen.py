import argparse
import os
import random
from enum import Enum

class AccessType(Enum):
    LOAD = "L"
    STORE = "S"

class TraceGen:
    def __init__(self, num_procs=4, cache_line_size=64, cache_size=8192, output_dir='traces_temp', seed=42):
        self.num_procs = num_procs
        self.cache_line_size = cache_line_size
        self.cache_size = cache_size
        self.output_dir = output_dir
        random.seed(seed)

        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

    def generate_false_sharing(self, num_accesses):
        for proc in range(self.num_procs):
            with open(f"{self.output_dir}/{proc}.txt", "w") as f:
                base_addr = 0
                offset = proc * 8
                addr = base_addr + offset
                
                recent_addresses = []
                
                for i in range(num_accesses):
                    if recent_addresses and random.random() < 0.7:
                        addr = random.choice(recent_addresses)
                    else:
                        addr = base_addr + offset
                    
                    access_type = AccessType.LOAD if i % 2 == 0 else AccessType.STORE
                    f.write(f"{access_type.value} {addr}\n")
                    
                    if len(recent_addresses) >= 3:
                        recent_addresses.pop(0)
                    recent_addresses.append(addr)

    def generate_no_sharing(self, num_accesses=10):
        for proc in range(self.num_procs):
            with open(f"{self.output_dir}/{proc}.txt", 'w') as f:
                base_addr = proc * 1000
                
                addr_pool = [base_addr + (i % 10) * self.cache_line_size for i in range(10)]
                recent_addresses = []
                
                for i in range(num_accesses):
                    if recent_addresses and random.random() < 0.7:
                        addr = random.choice(recent_addresses)
                    else:
                        addr = random.choice(addr_pool)
                    
                    access_type = AccessType.LOAD if i % 3 != 1 else AccessType.STORE
                    f.write(f"{access_type.value} {addr}\n")
                    
                    if len(recent_addresses) >= 3:
                        recent_addresses.pop(0)
                    recent_addresses.append(addr)
    
    def generate_producer_consumer(self, num_accesses=10):
        # Producer process (proc 0)
        with open(f"{self.output_dir}/0.txt", 'w') as f:
            addr_pool = [100 + i * self.cache_line_size for i in range(10)]
            recent_addresses = []
            
            for i in range(num_accesses):
                if recent_addresses and random.random() < 0.6:
                    addr = random.choice(recent_addresses)
                else:
                    addr = addr_pool[i % len(addr_pool)]
                
                f.write(f"{AccessType.STORE.value} {addr}\n")
                
                if len(recent_addresses) >= 3:
                    recent_addresses.pop(0)
                recent_addresses.append(addr)
        
        # Consumer processes
        for proc in range(1, self.num_procs):
            with open(f"{self.output_dir}/{proc}.txt", 'w') as f:
                addr_pool = [100 + i * self.cache_line_size for i in range(10)]
                recent_addresses = []
                
                for i in range(num_accesses):
                    if recent_addresses and random.random() < 0.7:
                        addr = random.choice(recent_addresses)
                    else:
                        addr = addr_pool[i % len(addr_pool)]
                    
                    f.write(f"{AccessType.LOAD.value} {addr}\n")
                    
                    if len(recent_addresses) >= 3:
                        recent_addresses.pop(0)
                    recent_addresses.append(addr)

    def generate_multiple_writers(self, num_accesses=10):
        for proc in range(self.num_procs):
            with open(f"{self.output_dir}/{proc}.txt", "w") as f:
                base_addr = 150
                addr_pool = [base_addr + i * 4 for i in range(3)]
                recent_addresses = []
                
                for i in range(num_accesses):
                    if recent_addresses and random.random() < 0.8:
                        addr = random.choice(recent_addresses)
                    else:
                        addr = random.choice(addr_pool)
                    
                    access_type = AccessType.LOAD if i % 2 == 0 else AccessType.STORE
                    f.write(f"{access_type.value} {addr}\n")
                    
                    if len(recent_addresses) >= 3:
                        recent_addresses.pop(0)
                    recent_addresses.append(addr)

    def generate_multiple_readers(self, num_accesses=10):
        for proc in range(self.num_procs):
            with open(f"{self.output_dir}/{proc}.txt", 'w') as f:
                base_addr = 200
                addr_pool = [base_addr + i * self.cache_line_size for i in range(5)]
                recent_addresses = []
                
                for i in range(num_accesses):
                    if recent_addresses and random.random() < 0.75:
                        addr = random.choice(recent_addresses)
                    else:
                        addr = random.choice(addr_pool)
                    
                    f.write(f"{AccessType.LOAD.value} {addr}\n")
                    
                    if len(recent_addresses) >= 3:
                        recent_addresses.pop(0)
                    recent_addresses.append(addr)

    def generate_random(self, num_accesses=10):
        addr_range = 512
        
        for proc in range(self.num_procs):
            with open(f"{self.output_dir}/{proc}.txt", 'w') as f:
                recent_addresses = []
                
                for i in range(num_accesses):
                    if recent_addresses and random.random() < 0.65:
                        addr = random.choice(recent_addresses)
                    else:
                        addr = random.randrange(0, addr_range, 4)
                    
                    access_type = random.choice([AccessType.LOAD, AccessType.STORE])
                    f.write(f"{access_type.value} {addr}\n")
                    
                    if len(recent_addresses) >= 5:
                        recent_addresses.pop(0)
                    recent_addresses.append(addr)

    def generate_partial_proc_use(self, num_accesses=10):
        active_procs = max(1, int(self.num_procs * 0.25))
        #active_procs = 1
        active_proc_indices = random.sample(range(self.num_procs), active_procs)
        shared_addr_base = 1000
        
        for proc in range(self.num_procs):
            with open(f"{self.output_dir}/{proc}.txt", 'w') as f:
                if proc in active_proc_indices:
                    addr_pool = [shared_addr_base + i * self.cache_line_size for i in range(5)]
                    recent_addresses = []
                    
                    for i in range(num_accesses):
                        if recent_addresses and random.random() < 0.7:
                            addr = random.choice(recent_addresses)
                        else:
                            addr = random.choice(addr_pool)
                        
                        access_type = AccessType.LOAD if i % 2 == 0 else AccessType.STORE
                        f.write(f"{access_type.value} {addr}\n")
                        
                        if len(recent_addresses) >= 3:
                            recent_addresses.pop(0)
                        recent_addresses.append(addr)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--pattern', choices=[
        "false_sharing",
        "producer_consumer",
        "multiple_writers",
        "multiple_readers",
        "no_sharing",
        "random",
        "partial_proc_use"
    ])

    parser.add_argument("--num-procs", type=int, default=4)
    parser.add_argument("--num-accesses", type=int, default=10)
    parser.add_argument("--output-dir", default='traces_temp')
    parser.add_argument("--cache-line", type=int, default=64)
    parser.add_argument("--cache-size", type=int, default=8192)
    parser.add_argument("--seed", type=int, default=42)

    args = parser.parse_args()

    generator = TraceGen(
        num_procs=args.num_procs,
        cache_line_size=args.cache_line,
        cache_size=args.cache_size,
        output_dir=args.output_dir,
        seed=args.seed
    )

    if args.pattern == "false_sharing":
        generator.generate_false_sharing(args.num_accesses)
    elif args.pattern == "producer_consumer":
        generator.generate_producer_consumer(args.num_accesses)
    elif args.pattern == "multiple_writers":
        generator.generate_multiple_writers(args.num_accesses)
    elif args.pattern == "multiple_readers":
        generator.generate_multiple_readers(args.num_accesses)
    elif args.pattern == "no_sharing":
        generator.generate_no_sharing(args.num_accesses)
    elif args.pattern == "random":
        generator.generate_random(args.num_accesses)
    elif args.pattern == "partial_proc_use":
        generator.generate_partial_proc_use(args.num_accesses)


if __name__ == "__main__":
    main()