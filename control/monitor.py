from argparse import ArgumentError
import ssl
from django.db.models import Avg
from datetime import timedelta, datetime
from receiver.models import Data, Measurement
import paho.mqtt.client as mqtt
import schedule
import time
from django.conf import settings


def generate_specific_alert(variable, current_value, min_value, max_value):
    """
    Genera comandos específicos de alerta según el tipo de variable y condición
    que el dispositivo IoT puede interpretar y procesar adecuadamente.
    """
    variable_lower = variable.lower()
    
    # Alertas específicas por tipo de variable
    if "temperatura" in variable_lower:
        if current_value > max_value:
            return "TEMP_HIGH"
        elif current_value < min_value:
            return "TEMP_LOW"  # Agregamos para temperatura baja
    
    elif "humedad" in variable_lower:
        if current_value > max_value:
            return "HUMIDITY_HIGH"
        elif current_value < min_value:
            return "HUMIDITY_LOW"  # Para humedad baja
    
    elif "luminosidad" in variable_lower or "luz" in variable_lower:
        if current_value < min_value:  # Luminosidad baja es más común
            return "LIGHT_LOW"
        elif current_value > max_value:
            return "LIGHT_HIGH"
    
    # Detección de anomalías estadísticas
    value_range = max_value - min_value
    if value_range > 0:
        deviation = abs(current_value - (max_value + min_value) / 2)
        if deviation > value_range * 0.8:  # Desviación mayor al 80% del rango
            return "ANOMALY"
    
    # Alertas de optimización energética
    if "temperatura" in variable_lower and current_value > max_value * 0.9:
        return "ENERGY_OPTIMIZE"
    
    # Estrés ambiental (múltiples variables fuera de rango)
    if current_value > max_value * 1.2 or current_value < min_value * 0.8:
        return "ENVIRONMENTAL_STRESS"
    
    # Alerta genérica como fallback
    return "ALERT {} {} {}".format(variable, min_value, max_value)


client = mqtt.Client(settings.MQTT_USER_PUB)


def analyze_data():
    # Consulta todos los datos de la última hora, los agrupa por estación y variable
    # Compara el promedio con los valores límite que están en la base de datos para esa variable.
    # Si el promedio se excede de los límites, se envia un mensaje de alerta.

    print("Calculando alertas...")

    data = Data.objects.filter(
        base_time__gte=datetime.now() - timedelta(hours=1))
    aggregation = data.annotate(check_value=Avg('avg_value')) \
        .select_related('station', 'measurement') \
        .select_related('station__user', 'station__location') \
        .select_related('station__location__city', 'station__location__state',
                        'station__location__country') \
        .values('check_value', 'station__user__username',
                'measurement__name',
                'measurement__max_value',
                'measurement__min_value',
                'station__location__city__name',
                'station__location__state__name',
                'station__location__country__name')
    alerts = 0
    normal_conditions = 0
    
    # Diccionario para rastrear el estado anterior de las alertas por estación
    station_alerts = {}
    
    for item in aggregation:
        alert = False

        variable = item["measurement__name"]
        max_value = item["measurement__max_value"] or 0
        min_value = item["measurement__min_value"] or 0
        current_value = item["check_value"]

        country = item['station__location__country__name']
        state = item['station__location__state__name']
        city = item['station__location__city__name']
        user = item['station__user__username']
        
        station_key = f"{country}/{state}/{city}/{user}"
        topic = '{}/{}/{}/{}/in'.format(country, state, city, user)

        # Verificar si hay condición de alerta
        if current_value > max_value or current_value < min_value:
            alert = True
            # Generar mensaje específico según el tipo de variable y condición
            message = generate_specific_alert(variable, current_value, min_value, max_value)
            print(datetime.now(), "Sending alert to {} {}: {}".format(topic, variable, message))
            client.publish(topic, message)
            alerts += 1
            station_alerts[station_key] = True
        else:
            # Condiciones normales - enviar ALERT_OFF si había alerta previa
            if station_key in station_alerts and station_alerts[station_key]:
                print(datetime.now(), "Sending ALERT_OFF to {} {}".format(topic, variable))
                client.publish(topic, "ALERT_OFF")
                normal_conditions += 1
            station_alerts[station_key] = False

    print(len(aggregation), "dispositivos revisados")
    print(alerts, "alertas enviadas")
    print(normal_conditions, "alertas desactivadas")


def on_connect(client, userdata, flags, rc):
    '''
    Función que se ejecuta cuando se conecta al bróker.
    '''
    print("Conectando al broker MQTT...", mqtt.connack_string(rc))


def on_disconnect(client: mqtt.Client, userdata, rc):
    '''
    Función que se ejecuta cuando se desconecta del broker.
    Intenta reconectar al bróker.
    '''
    print("Desconectado con mensaje:" + str(mqtt.connack_string(rc)))
    print("Reconectando...")
    client.reconnect()


def setup_mqtt():
    '''
    Configura el cliente MQTT para conectarse al broker.
    '''

    print("Iniciando cliente MQTT...", settings.MQTT_HOST, settings.MQTT_PORT)
    global client
    try:
        client = mqtt.Client(settings.MQTT_USER_PUB)
        client.on_connect = on_connect
        client.on_disconnect = on_disconnect

        if settings.MQTT_USE_TLS:
            client.tls_set(ca_certs=settings.CA_CRT_PATH,
                           tls_version=ssl.PROTOCOL_TLSv1_2, cert_reqs=ssl.CERT_NONE)

        client.username_pw_set(settings.MQTT_USER_PUB,
                               settings.MQTT_PASSWORD_PUB)
        client.connect(settings.MQTT_HOST, settings.MQTT_PORT)

    except Exception as e:
        print('Ocurrió un error al conectar con el bróker MQTT:', e)


def start_cron():
    '''
    Inicia el cron que se encarga de ejecutar la función analyze_data cada 5 minutos.
    '''
    print("Iniciando cron...")
    schedule.every(5).minutes.do(analyze_data)
    print("Servicio de control iniciado")
    while 1:
        schedule.run_pending()
        time.sleep(1)
