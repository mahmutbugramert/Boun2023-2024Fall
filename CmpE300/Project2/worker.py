from mpi4py import MPI


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


def add(products):  # concatenates list of products
    product = ""
    for i in products:
        product = product + i
    return product


def enhance(product):  # duplicates first and last letters of the product
    product = product[0] + product
    product = product + product[len(product) - 1]
    return product


def reverse(product):  # reverses the product
    product = product[::-1]
    return product


def chop(product):  # removes last letter of the product
    if len(product) == 1:
        return product
    else:
        product = product[: len(product) - 1 :]
        return product


def trim(product):  # removes first and last letter of the product
    if len(product) == 1 or len(product) == 2:
        return product
    else:
        product = product[1::]
        product = chop(product)
        return product


def split(product):  # splits the product in two
    if len(product) % 2 == 0:
        product = product[: len(product) // 2 :]
    else:
        product = product[: len(product) // 2 + 1 :]
    return product


def operation(type, product):  # apply the correct operation type to the product
    if type == "enhance":
        return enhance(product)
    if type == "reverse":
        return reverse(product)
    if type == "chop":
        return chop(product)
    if type == "trim":
        return trim(product)
    if type == "split":
        return split(product)
    if type == "add":
        return add(product)


def wear(type, cost):  # returns the wear of the operation
    if type == "enhance":
        return cost["enhance"]
    if type == "reverse":
        return cost["reverse"]
    if type == "chop":
        return cost["chop"]
    if type == "trim":
        return cost["trim"]
    if type == "split":
        return cost["split"]
    if type == "add":
        return 0


def cost_of_machine(type, machine, cost):  # returns the cost of the machine
    if machine.wear < cost["threshold"]:
        return 0
    else:
        return (machine.wear - cost["threshold"] + 1) * cost[type]


def change_type(my_machine):  # to change the type of operation for next cycle
    if my_machine.id % 2 == 0:
        if my_machine.type == "split":
            my_machine.set_type("chop")
            machines[my_machine.id] = my_machine
        elif my_machine.type == "chop":
            my_machine.set_type("enhance")
            machines[my_machine.id] = my_machine
        elif my_machine.type == "enhance":
            my_machine.set_type("split")
            machines[my_machine.id] = my_machine
    else:
        if my_machine.type == "trim":
            my_machine.set_type("reverse")
            machines[my_machine.id] = my_machine
        elif my_machine.type == "reverse":
            my_machine.set_type("trim")
            machines[my_machine.id] = my_machine


parcomm = MPI.Comm.Get_parent()  # for communication with parent
comm = MPI.COMM_WORLD  # for communication between spawned processes

size = comm.Get_size()  # no of processes
rank = comm.Get_rank()  # rank of processes

received_data = parcomm.recv(source=0, tag=0)  # recieve dictionary from main center
machine_id = rank + 1  # ranks starts from 0. So add 1 to have same machine_ids

# pull data from dictionary
leafs = received_data["leafs"]
number_of_prod_cyc = received_data["number_of_prod_cyc"]
machines = received_data["machines"]
cost = received_data["cost"]
threshold = cost["threshold"]


for j in range(
    number_of_prod_cyc
):  # do the operations number of productions cycle times
    my_machine = machines[machine_id]
    my_type = my_machine.type
    my_product = my_machine.product
    my_parent_rank = my_machine.pid - 1  # same reason above
    # print("rank:", rank, "type:", my_type)
    if my_machine.is_leaf == True:  # leaf machines operations
        my_inputs = []
        my_inputs.append(my_product)
        # print(my_inputs)
        my_product = add(my_inputs)  # first do add operation
        my_output = operation(my_type, my_product)  # then do its type of operation
        # print("myparentrank: ", my_parent_rank, "merank: ", machine_id - 2)
        # comm.send((my_output, my_parent_rank), dest = 0, tag = 1)

        my_wear = wear(my_type, cost)  # calculate wear of the operation
        my_machine.set_wear(my_machine.wear + my_wear)  # add wear to the machine
        parcomm.send(
            cost_of_machine(my_type, my_machine, cost), dest=0, tag=2
        )  # send cost of the machine to control room
        if my_machine.wear >= cost["threshold"]:  # if wear is higher than threshold
            my_machine.set_wear(0)  # reset wear
        comm.send(
            my_output, dest=my_parent_rank, tag=1
        )  # send output of operation the parent of the machine
        change_type(my_machine)  # change the type of operation for next cycle
        comm.Barrier()  # this makes all processes to wait here for other processes before passing to the next cycle

    else:  # for non-leaf machines
        my_inputs = []
        my_childs = my_machine.child_machines
        # print("nonleafrank:", rank)
        # print(my_childs)
        # print(my_type)
        for i in my_childs:  # gather all inputs from child processes
            input_data = comm.recv(source=i - 1, tag=1)
            my_inputs.append(input_data)

        my_product = add(my_inputs)  # do add operation first
        my_output = operation(
            my_type, my_product
        )  # then do the specific type of operation

        my_wear = wear(my_type, cost)  # calculate wear of the operation
        my_machine.set_wear(my_machine.wear + my_wear)  # add wear to the machine
        parcomm.send(
            cost_of_machine(my_type, my_machine, cost), dest=0, tag=2
        )  # send cost of the machine to main center
        if my_machine.wear >= cost["threshold"]:  # if wear is higher than threshold
            my_machine.set_wear(0)  # reset wear

        if (
            rank == 0
        ):  # if the machine is terminal machine send result to main center via using parcomm
            parcomm.send(my_output, dest=0, tag=0)
        else:  # if not, then send the output to the parent of the current machine
            comm.send(my_output, dest=my_parent_rank, tag=1)
        change_type(my_machine)  # change the type of operation for next cycle
        comm.Barrier()  # this makes all processes to wait here for other processes before passing to the next cycle
