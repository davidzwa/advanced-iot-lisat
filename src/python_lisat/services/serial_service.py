import asyncio
from typing import Any, Coroutine

from aioserial import AioSerial


class SerialService(object):
    serial_messages = []
    iterator = 0

    def __init__(self, port, baudrate):
        self.port = port
        self.baudrate = baudrate
        self.serial_async: AioSerial = None

    def setup_serial(self):
        # https://github.com/vxgmichel/aioconsole/blob/master/example/dice.py
        self.serial_async = AioSerial(port=self.port, baudrate=self.baudrate)

    async def fetch_update(self, *args):
        print("-- Pushing 'info' command.")
        asyncio.ensure_future(self.serial_async.write_async(b"fetch\r"))
        print("-- Command pushed.")

    @asyncio.coroutine
    def fetch_regular_update(self):
        while True:
            self.iterator += 1
            print("Fetching device update #{}.".format(self.iterator))
            encoded_data = "info{}\r".format(self.iterator).encode()
            asyncio.ensure_future(
                self.serial_async.write_async(encoded_data)
            )
            yield from asyncio.sleep(5)

    def get_inputhandler_coroutine(self) -> Coroutine[Any, Any, None]:
        return self.read_and_print(self.serial_async)

    async def read_and_print(self, aioserial_instance: AioSerial):
        while True:
            recv_input = await aioserial_instance.readline_async()
            decoded_input = recv_input.decode(errors='ignore')
            self.serial_messages.append((decoded_input))
            print('Data length received:', len(decoded_input))
            print(">>{}<<".format(decoded_input.rstrip()), flush=True)
