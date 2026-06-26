import json
import boto3

# Khai báo kết nối Database và Loa phát thanh SNS
dynamodb = boto3.resource('dynamodb')
table = dynamodb.Table('VisionBin_Status')
sns = boto3.client('sns')

# DÁN MÃ ARN CỦA BẠN VÀO GIỮA 2 DẤU NGOẶC KÉP NÀY:
SNS_TOPIC_ARN = 'arn:aws:sns:ap-southeast-1:863956449235:TrashAlertTopic'

def lambda_handler(event, context):
    try:
        body = json.loads(event['body'])
        bin_id = body['bin_id']
        fill_level = int(body['fill_level']) # Ép kiểu về số nguyên để kiểm tra
        
        # 1. Lưu vào Database như cũ
        table.put_item(
            Item={
                'bin_id': bin_id,
                'fill_level': fill_level
            }
        )
        
        # 2. LOGIC BÁO ĐỘNG ĐỎ: Nếu rác >= 90% thì gửi Email
        if fill_level >= 90:
            message = f"🚨 CẢNH BÁO KHẨN CẤP: Thùng rác {bin_id} đã đầy {fill_level}%. Yêu cầu bộ phận vệ sinh điều phối người đến thu gom ngay lập tức để tránh ô nhiễm!"
            
            sns.publish(
                TopicArn=SNS_TOPIC_ARN,
                Message=message,
                Subject=f"[Neo League] Báo động rác đầy - {bin_id}"
            )
            
        return {
            'statusCode': 200,
            'body': json.dumps('Cập nhật và kiểm tra cảnh báo thành công!')
        }
    except Exception as e:
        return {
            'statusCode': 500,
            'body': json.dumps(f'Lỗi rùi: {str(e)}')
        }