import json
import boto3

# Kết nối với cái bảng DynamoDB vừa tạo
dynamodb = boto3.resource('dynamodb')
table = dynamodb.Table('VisionBin_Status')

def lambda_handler(event, context):
    try:
        # Lấy dữ liệu do mạch ESP32 của Công gửi lên
        body = json.loads(event['body'])
        bin_id = body['bin_id']
        fill_level = body['fill_level'] # % rác
        
        # Lưu vào database
        table.put_item(
            Item={
                'bin_id': bin_id,
                'fill_level': fill_level
            }
        )
        return {
            'statusCode': 200,
            'body': json.dumps('Cập nhật % rác thành công!')
        }
    except Exception as e:
        return {
            'statusCode': 500,
            'body': json.dumps(f'Lỗi rùi: {str(e)}')
        }