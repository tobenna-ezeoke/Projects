import requests
from selenium import webdriver
from selenium.webdriver.common.by import By
import time

# URLs for Firebase data
firebase_bin_status_url = "https://sotta-bin-default-rtdb.firebaseio.com/bin_status.json"
firebase_classification_url = "https://sotta-bin-default-rtdb.firebaseio.com/classification_data.json"

# URL for the local website (running on your local machine)
website_bin_status_url = "http://127.0.0.1:8081/bin-details.html"
website_classification_url = "http://127.0.0.1:8081/classification.html"

# Function to fetch data from Firebase
def get_firebase_data(url):
    try:
        response = requests.get(url)
        response.raise_for_status()  # Will raise an HTTPError for bad responses
        return response.json()  # Return JSON response
    except requests.exceptions.RequestException as e:
        print(f"Error fetching Firebase data from {url}: {e}")
        return None

# Function to fetch data from the website using Selenium
def get_website_data(url, xpath):
    # Use a ChromeDriver to interact with the webpage
    driver = webdriver.Chrome()  # Ensure you have the correct path to your chromedriver
    driver.get(url)

    time.sleep(3)  # Wait for page to load completely
    
    
    
    driver.quit()  # Close the browser

# Compare the data
def compare_data(firebase_data, website_data, data_type):
    if firebase_data == firebase_data:
        print(f"{data_type} data matches!")
    else:
        print(f"{data_type} data mismatch!")
        print("Firebase Data:", firebase_data)
        print("Website Data:", website_data)

# Fetch data from Firebase
firebase_bin_status = get_firebase_data(firebase_bin_status_url)
firebase_classification = get_firebase_data(firebase_classification_url)
print(firebase_bin_status)
print(firebase_classification)

# Define the XPath for the data you want to extract from the website
bin_status_xpath = "//div[@id='bin-status']//h3"  # Adjust this based on your HTML structure
classification_xpath = "//div[@id='classification-data']//p"  # Adjust this based on your HTML structure

# Fetch data from Website (using Selenium)
website_bin_status = get_website_data(website_bin_status_url, bin_status_xpath)
website_classification = get_website_data(website_classification_url, classification_xpath)
print(firebase_bin_status)
print(firebase_classification)
# Compare bin status

compare_data(firebase_bin_status, website_bin_status, "Bin Status")

# Compare classification data

compare_data(firebase_classification, website_classification, "Classification")
