import json
import boto3

dynamodb = boto3.resource('dynamodb')
table = dynamodb.Table('VisionBin_Status')

def lambda_handler(event, context):
    try:
        # Lệnh scan() sẽ quét và lấy toàn bộ dữ liệu có trong bảng
        response = table.scan()
        items = response.get('Items', [])
        
        # Xử lý định dạng số cho tương thích với JSON
        for item in items:
            item['fill_level'] = int(item['fill_level'])
            
        return {
            'statusCode': 200,
            'body': json.dumps(items)
        }
    except Exception as e:
        return {
            'statusCode': 500,
            'body': json.dumps(f'Lỗi rùi: {str(e)}')
        }