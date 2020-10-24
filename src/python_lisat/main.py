"""
LISAT Command-line interface and system debugger
Created by: David Zwart

Description:    a multitool for receiving serial data from an ESP8266, MSP432 and MQTT messages from a private mosquitto
                server. Contains a streaming CLI to interact with it in a 'argparse' way (strict cli interface).
"""
import argparse
import asyncio
import random

from aioconsole import AsynchronousCli
from python_lisat.constants import *
from python_lisat.services.serial_service import SerialService


async def dice(reader, writer, faces, *args, **kwargs):
    for _ in range(3):
        await asyncio.sleep(0.33)
        writer.write(".")
        await writer.drain()
    writer.write("\n")
    return random.randint(1, faces)


async def fetch_device_info(reader, writer, *args, **kwargs):
    print("Fetching device info")
    for _ in range(3):
        await asyncio.sleep(1)
        writer.write(".")
        await writer.drain()
    writer.write("Device too dumb to respond.")


def main():
    parser = argparse.ArgumentParser(description="Understand this CLI.")
    parser.add_argument(
        "--faces", "-f", metavar="N", type=int, default=6, help="Number of faces"
    )
    parser2 = argparse.ArgumentParser(description="Command your serial microcontroller or MQTT client.")

    serialHandler = SerialService(PORT, BAUDRATE)
    serialHandler.setup_serial()

    cli_commands = {
        "example": (dice, parser),
        "fetch": (serialHandler.fetch_update, parser2)
    }
    # https://github.com/vxgmichel/aioconsole/blob/master/example/dice.py
    asyncCliHandler = AsynchronousCli(cli_commands, prog = PROGRAM_NAME)

    loop = asyncio.get_event_loop()
    asyncio.run_coroutine_threadsafe(serialHandler.fetch_regular_update(), loop=loop)
    asyncio.run_coroutine_threadsafe(serialHandler.get_inputhandler_coroutine(), loop=loop)
    loop.run_until_complete(asyncCliHandler.interact())


if __name__ == "__main__":
    main()
