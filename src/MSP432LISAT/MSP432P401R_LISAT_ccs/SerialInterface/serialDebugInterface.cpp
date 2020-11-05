#include <SerialInterface/serialDebugInterface.h>

// States
bool transmitting = false;

void transmitEndSymbol()
{
    Display_printf(display, 0, 0, "--Done");
}

void transmitSerialData(transmittedData_t *data)
{
    transmitting = true;
    for (int16_t i = 0; i < data->length; i++)
    {
        transmitSerialValue(data->buffer[i]);
    }
    transmitSerialParam('M', '1', data->deltaMic1);
    transmitSerialParam('M', '2', data->deltaMic2);
    transmitSerialParam('M', '3', data->deltaMic3);
    transmitSerialParam('D', '1', data->tdoaDir1);
    transmitSerialParam('D', '2', data->tdoaDir2);
    transmitSerialParam('F', 's', SAMPLE_FREQUENCY);
    transmitEndSymbol(); // End of data separator
    transmitting = false;
}

// Parameters dont get sent when in STREAM mode
void transmitSerialParam(char identifier, char identifier2, int value)
{
#ifndef STREAM_PLOTTER
    Display_printf(display, 0, 0, "%c%c.%d", identifier, identifier2, value);
#endif
}

void transmitSerialValue(int value)
{
#ifdef STREAM_PLOTTER
    Display_printf(display, 0, 0, "v = %d", value);
#else
    Display_printf(display, 0, 0, "v.%d\r\n", value);
#endif
}
