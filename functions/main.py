import logging
import os
from datetime import datetime 
from flask import Response, Request
from google.cloud import bigquery

dataset_id = 'mkr1010_dataset'
table_name = 'raw_data'
project_id = os.getenv('GCLOUD_PROJECT')
client = bigquery.Client()

def insert_bigquery(data):    
    dataset_ref = client.dataset(dataset_id, project=project_id)    
    table_ref = dataset_ref.table(table_name)
    table = client.get_table(table_ref)

    rows_to_insert = [
        (
            data['device_id'], 
            data['timestamp'],
            data['red'], 
            data['blue'], 
            data['green'], 
            data['light'],
        )
    ]    
    client.insert_rows(table,rows_to_insert)

def telemetry_handler(request):
    '''
    :param request:
    :type request: Request
    '''
    logging.info('[telemetry_handler] started.')
    
    logging.info('[telemetry_handler][parse_data] started.')    
    received_data = {      
      "device_id": request.args.get('deviceId'),
      "red": request.args.get('red'),
      "green": request.args.get('green'),
      "blue": request.args.get('blue'),
      "light": request.args.get('light'),
      "timestamp": str(datetime.now())
    }        
    logging.info('Received data : %s', received_data)
    logging.info('[telemetry_handler][parse_data] ended.')

    logging.info('[telemetry_handler][insert_bigquery] started.')    
    insert_bigquery(received_data)
    logging.info('[telemetry_handler][insert_bigquery] ended.')

    logging.info('[telemetry_handler] finished.')
