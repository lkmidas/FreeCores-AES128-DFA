import subprocess
import phoenixAES
import aeskeyschedule

# To be consistent with the aes_128 implementation and the tools used, these are the names that I used in this PoC:
# The 10 rounds are: R1, R2, R3, ..., R9, RF (with RF being the final 10th round)
# The 11 round keys are: K00, K01, K02, ..., K10 (the aes_128 implementation named them as: key, k0b, k1b, ..., k9b)


# Run Verilator simulation to collect faulty output in fault.txt
print("[+] Running simulation")
subprocess.run(['./simulation/build/fault_injection'])

# Recover round key K10 from fault.txt
print("[+] Running DFA against AES-128")
k10 = bytes.fromhex(phoenixAES.crack_file("fault.txt"))

# Recover cipher key K from round key K10 be reversing AES key schedule algorithm
print("[+] Running reverse key schedule from K10")
base_key = aeskeyschedule.reverse_key_schedule(k10, 10)
print("Cipher key found:")
print(base_key.hex())

print("[+] Finished")
