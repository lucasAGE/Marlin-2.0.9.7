#ifndef PCF8574_H
#define PCF8574_H

/**
 * pcf8574.h
 * 
 * Driver genérico para o expansor de I²C PCF8574, utilizado para controlar
 * até 8 pinos de saída (por exemplo, MOSFETs de camas aquecidas).
 * 
 * Funcionalidades básicas:
 *  - begin(): inicializa o barramento I²C e armazena o endereço
 *  - writeBit(pin, state): modifica um único bit no registrador de saída
 *  - writeMask(mask): escreve todos os 8 bits de uma só vez
 *  - readPins(): lê o estado atual dos 8 pinos (se necessário para entrada)
 * 
 * O driver mantém um cache (uint8_t _portState) do último valor enviado,
 * para facilitar operações de read-modify-write sem afetar outros bits.
 */

#include <Arduino.h>
#include <Wire.h>

class Pcf8574 {
public:
  /**
   * Construtor.
   * @param address Endereço I²C do PCF8574 (0x20 a 0x27, dependendo dos pinos A0–A2).
   */
  explicit Pcf8574(uint8_t address = 0x20);

  /**
   * Inicializa o hardware I²C (Wire) e configura o endereço do PCF8574.
   * Deve ser chamado antes de qualquer operação de escrita/leitura.
   * 
   * @param wirePort Referência ao objeto TwoWire (por exemplo, Wire). 
   *                 Caso não seja passado, usa Wire por padrão.
   */
  void begin(TwoWire &wirePort = Wire);

  /**
   * Escreve todos os 8 bits de uma vez no PCF8574.
   * @param mask Byte completo com o estado de todos os pinos (1 = HIGH, 0 = LOW).
   */
  void writeMask(uint8_t mask);

  /**
   * Modifica apenas um único bit (pino) no PCF8574, preservando os demais.
   * @param pin   Número do pino [0..7] correspondente a P0..P7 do PCF8574.
   * @param state true para colocar HIGH, false para LOW.
   */
  void writeBit(uint8_t pin, bool state);

  /**
   * Lê o estado atual dos 8 pinos do PCF8574.
   * Obs.: Se os pinos estiverem configurados como saída, retorna o valor
   *       que estiver atualmente trancado no latch interno. Se estiverem
   *       como entrada, reflete o nível físico do pino.
   * @return Byte com o estado dos 8 pinos (bit 0 = P0, bit 7 = P7).
   */
  uint8_t readPins();

private:
  TwoWire * _wire;      ///< Ponteiro para o objeto I²C (Wire)
  uint8_t   _address;   ///< Endereço I²C do PCF8574
  uint8_t   _portState; ///< Cache local do último byte escrito no PCF8574

  /**
   * Envia o byte _portState para o PCF8574 via I²C.
   */
  void _writeToHardware();
};

#endif // PCF8574_H
