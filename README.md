# IOT Monitoring Server

Sistema completo de monitoreo IoT con Django, MQTT y dispositivos ESP32.

## 🔧 Componentes del Sistema

- **Servidor Django**: Interfaz web y API
- **Broker MQTT**: Comunicación con dispositivos IoT
- **PostgreSQL**: Base de datos de series temporales
- **ESP32 + Sensores**: Dispositivos IoT con DHT11, LDR y OLED

## 📋 Instalación

### 1. Dependencias Python
```bash
pip install -r requirements.txt
```

### 2. Base de datos
Configurar PostgreSQL con las credenciales en `settings.py`:
```python
"HOST": "54.167.96.107",
"PORT": "5432",
"NAME": "iot_data",
"USER": "dbadmin",
"PASSWORD": "uniandesIOT1234*"
```

### 3. Broker MQTT
Configurar broker MQTT en `settings.py`:
```python
MQTT_HOST = "32.192.84.37"
MQTT_PORT = 8082
```

## 🚀 Ejecución

### 1. Migrar base de datos
```bash
python manage.py migrate
```

### 2. Crear superusuario
```bash
python manage.py createsuperuser
```

### 3. Iniciar servicios (en terminales separados)

#### Servidor web:
```bash
python manage.py runserver
```

#### Servicio de recepción MQTT:
```bash
python manage.py start_mqtt
```

#### Servicio de control y alertas:
```bash
python manage.py start_control
```

## 📱 Dispositivo IoT (ESP32)

### Hardware requerido:
- ESP32 DevKit
- Sensor DHT11 (Pin 4)
- Sensor LDR (Pin 15)
- Pantalla OLED SH1106 (I2C: SDA=22, SCL=21)
- LED integrado (Pin 2)

### Configuración WiFi y MQTT:
Editar en `hardware/iot_device_with_event_processing.ino`:
```cpp
const char ssid[] = "TU_WIFI";
const char pass[] = "TU_PASSWORD";
#define USER "tu.usuario"
```

### Comandos soportados:
- `TEMP_HIGH`: Temperatura alta
- `HUMIDITY_HIGH`: Humedad alta
- `LIGHT_LOW`: Luminosidad baja
- `ANOMALY`: Anomalía detectada
- `ENERGY_OPTIMIZE`: Optimizar energía
- `ENVIRONMENTAL_STRESS`: Estrés ambiental
- `ALERT_OFF`: Desactivar alertas

## 🌐 Interfaz Web

Acceder a `http://localhost:8000`:

- `/` - Dashboard principal
- `/realtime-data/` - Datos en tiempo real
- `/map/` - Mapa de estaciones
- `/historic/` - Datos históricos
- `/users/` - Gestión de usuarios (admin)
- `/variables/` - Gestión de variables (admin)

## 🔧 Comandos útiles

### Enviar comando manual a dispositivo:
```bash
python manage.py send_command --user jn.cordobap1 --city ciudad --state estado --country pais --command TEMP_HIGH
```

### Ver logs MQTT:
```bash
python manage.py start_mqtt
```

### Monitorear alertas:
```bash
python manage.py start_control
```

## 📊 Estructura de datos MQTT

### Envío desde dispositivo (`/out`):
```json
{
    "temperatura": 25.5,
    "humedad": 60.2,
    "luminosidad": 350
}
```

### Comandos hacia dispositivo (`/in`):
```
TEMP_HIGH
HUMIDITY_HIGH
LIGHT_LOW
ANOMALY
ENERGY_OPTIMIZE
ENVIRONMENTAL_STRESS
ALERT_OFF
```

## 🔒 Seguridad

- Cambiar `SECRET_KEY` en producción
- Configurar `DEBUG = False` en producción
- Usar variables de entorno para credenciales
- Habilitar TLS para MQTT si es necesario

## 🐛 Troubleshooting

### Error de conexión MQTT:
- Verificar credenciales en `settings.py`
- Comprobar conectividad al broker

### Error de base de datos:
- Verificar configuración PostgreSQL
- Ejecutar migraciones

### Dispositivo no aparece:
- Verificar configuración WiFi en .ino
- Comprobar credenciales MQTT
- Revisar tópicos MQTT

## 📝 Logs

El sistema genera logs detallados en:
- Console del servicio MQTT
- Console del servicio de control
- Monitor Serial del ESP32