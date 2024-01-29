from mpi4py import MPI
import sys


# Group 3
# Mahmut Buğra Mert 2020400168
# Kamil Deniz Coşkuneer 2020400192
class Machine:  # machine object
    def __init__(self, id, pid, type):
        self.id = id  # machine id
        self.pid = pid  # parent id
        self.type = type  # operation type
        self.child_machines = []  # list of child machines ids
        self.product = None  # product name of leafs
        self.is_leaf = False  # is machine a leaf meachine
        self.wear = 0  # wear of the machine

    def set_product(self, product):
        self.product = product

    def add_child(self, child_id):  # to add child to child list
        self.child_machines.append(child_id)

    def set_type(self, type):
        self.type = type

    def set_wear(self, wear):
        self.wear = wear


input_file = open(sys.argv[1], "r")
output_file = open(sys.argv[2], "w")

# read inputs first 4 lines
number_of_machines = int(input_file.readline().strip())
number_of_prod_cyc = int(input_file.readline().strip())
wear_factors = input_file.readline().strip().split(" ")
wf_enhance = int(wear_factors[0])
wf_reverse = int(wear_factors[1])
wf_chop = int(wear_factors[2])
wf_trim = int(wear_factors[3])
wf_split = int(wear_factors[4])
threshold = int(input_file.readline().strip())

machines = {}
leafs = []

# add first machine to dictionary
new_machine = Machine(id=1, pid=0, type="add")
machines[new_machine.id] = new_machine
# read machines from input file
for i in range(number_of_machines - 1):
    machine_info = input_file.readline().strip().split(" ")
    # create machine objects
    new_machine = Machine(
        id=int(machine_info[0]), pid=int(machine_info[1]), type=machine_info[2]
    )
    # add machine objects to a dictionary
    machines[new_machine.id] = new_machine
    # add ids to a leafs list
    leafs.append(new_machine.id)
    # add this machine as a child to its parent machine
    parent_machine = machines[int(machine_info[1])]
    parent_machine.add_child(child_id=new_machine.id)
    machines.update({int(machine_info[1]): parent_machine})
    # if an id added to the leafs list is a pid, then remove that id from the list
    try:
        leafs.remove(new_machine.pid)
    except:
        continue
# for m in range(1, number_of_machines  + 1):
#     print(machines[m].id, machines[m].pid, machines[m].child_machines)
leafs.sort()
for j in leafs:
    # set the products of the leafs
    machines[j].set_product(product=input_file.readline().strip())
    machines[j].is_leaf = True
# print(leafs)

parcomm = MPI.COMM_SELF.Spawn(
    "python", args=["worker.py"], maxprocs=number_of_machines
)  # for communication between control room and spawned processes

cost = {
    "enhance": wf_enhance,
    "reverse": wf_reverse,
    "chop": wf_chop,
    "split": wf_split,
    "trim": wf_trim,
    "threshold": threshold,
}  # dictionary to keep costs of machines
broadcast_data = {
    "number_of_machines": number_of_machines,  # important datas to send spawned machines
    "number_of_prod_cyc": number_of_prod_cyc,
    "cost": cost,
    "machines": machines,
    "leafs": leafs,
}

for i in range(number_of_machines):  # send data to all machines
    parcomm.send(broadcast_data, dest=i, tag=0)

maintanance_logs = []  # list of maintanance logs
for n in range(number_of_prod_cyc):
    data = parcomm.recv(source=0, tag=0)
    # print(f"final output is: {data}")
    output_file.write(data + "\n")
    for machine in machines.values():
        maintanence_cost = parcomm.recv(source=machine.id - 1, tag=2)
        if maintanence_cost != 0:
            maintanance_logs.append(f"{machine.id}-{maintanence_cost}-{n + 1}")


def custom_sort(s):
    return int(s.split("-")[0])


sorted_maintanance_logs = sorted(maintanance_logs, key=custom_sort)
for log in sorted_maintanance_logs[: len(sorted_maintanance_logs) - 1]:
    output_file.write(log + "\n")
output_file.write(sorted_maintanance_logs[len(sorted_maintanance_logs) - 1])

input_file.close()
output_file.close()
