# DOTA_STM32_ESP8266
## Why FOTA?

Modern vehicles contain hundreds or even thousands of ECUs (Electronic Control Units). When updates, bug fixes, or new features need to be added, performing these upgrades in a maintenance center can be costly and require specialized equipment. FOTA (Firmware Over-The-Air) addresses these challenges by enabling wireless, efficient, and cost-effective firmware updates for numerous ECUs in vehicles.

## Benefits of FOTA

- **Remote Updates**: FOTA enables firmware updates to be performed remotely.
- **Cost Savings**: FOTA eliminates the need for on-site visits or manual interventions by technicians.
- **Scalability**: FOTA is highly scalable, allowing updates to be delivered to a large number of devices simultaneously.
- **Bug Fixes and Improvements**: FOTA provides a streamlined approach to deliver bug fixes, patches, and new features to embedded systems.



## CHALLENGES

- General protocol to send updates to system.
- Reduce updated data as possible --> Limited resources & Save data.
- Send updated data in a secure way-->Attacker may sniff data & Send modify data from attackers


## Bootloader

- **Manage Services Needed**: The bootloader manages essential services required for FOTA operations.
- **Receive Frames in Specific Sequence**: It receives data frames in a defined sequence to ensure reliable updates.
- **Respond to Frames Sent**: The bootloader acknowledges frames sent, facilitating communication throughout the update process.
- **Frame Style**:
-
![FOTA Process](/img/frame_bootloader.png)

## AES

AES (Advanced Encryption Standard) is a widely trusted encryption algorithm, adopted as the standard by the U.S. government and many organizations around the world. It supports key sizes of 128, 192, and 256 bits. In this project, we use a 128-bit key, which involves 10 rounds of encryption.

### Operations in AES (128-bit)

- **Add Round Key**: Combines each byte of the state with a portion of the expanded key.
- **Sub-Bytes**: Applies a non-linear substitution to each byte using a substitution table.
- **Shift Rows**: Shifts rows in the state to the left, with shifts varying per row.
- **Mix Columns**: Mixes data within each column to provide diffusion.

## Security of a 128-Bit Key

Cracking a 128-bit AES encryption key is extremely difficult and practically unfeasible with current technology. To put it into perspective:

- **Brute Force Attack Time**: It would take approximately **1 billion years** to crack a 128-bit AES key using brute force.
- **Comparison to DES**: A machine that can crack a DES (56-bit) key in a single second would still require **149 trillion years** to break a 128-bit AES key.

This level of security makes AES a reliable choice for protecting sensitive information in embedded systems and other applications.

## Disadvantages of AES

- **Complex Key Management**: Proper key management is essential but challenging. It requires secure generation, distribution, and storage of encryption keys.
- **High Computational Demand**: AES demands substantial computational power, particularly for larger key sizes (192-bit and 256-bit), which can strain resource-constrained systems.
- **Data Size Limitations**: AES operates on fixed block sizes (128 bits). For very large data streams, additional processing is required to manage data efficiently, which may impact performance.

## Patching

### Advantages

- **Vulnerability Fixes**: Patching addresses and fixes known vulnerabilities, helping to protect devices from potential exploits.
- **Optimized Code**: Updates often include performance enhancements, leading to faster and more efficient device operation.
- **Bug Fixes**: Resolves issues that may cause devices to malfunction or operate sub-optimally.
- **Longevity**: Keeps devices up-to-date with the latest advancements, extending their lifespan and delaying the need for replacements.
- **Reduced Update Size**: Minimizes the overall data required for system updates, which is particularly beneficial in resource-constrained environments (a key advantage for this project).

## Patching Sections

- **App1**: The first application to run after the bootloader, located at address `0x08008000`, with a size of 33 KB.
- **Patching Application**: Located at address `0x08010400`, with a size of 20 KB. This application is responsible for applying patches as needed.
- **App2**: A secondary application located at address `0x08015400`, with a size of 33 KB.


![Patching Sections](/img/SECTIONS.png)

## YouTube Playlist in Arabic

For more detailed information, you can watch the [YouTube playlist in Arabic](https://www.youtube.com/playlist?list=PLAaD5nt6MJGQ31_0qnO1lXGpwnUUKgoiM).
