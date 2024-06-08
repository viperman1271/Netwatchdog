import { redirectToLogin } from './redirectToLogin.js'

export function checkTokenAndRedirect() {
    const token = localStorage.getItem('authToken');
    if (token) {
        fetch('/api/protected', {
            method: 'GET',
            headers: {
                'Authorization': 'Bearer ' + token
            }
        })
        .then(response => {
            if (response.ok) {
                return response.text();
            } else {
                throw new Error('Invalid token');
            }
        })
        .then(data => {
            // Optionally, display the data on the page
            const resultDiv = document.getElementById('result');
            if (resultDiv) {
                resultDiv.textContent = data;
            }

            const usernameElem = document.getElementById('username');
            if(usernameElem) {
                const jsonObject = JSON.parse(data);
                var usernameText = jsonObject['username'];

                const length = usernameText.length;
                if(length > 16) {
                    usernameText = usernameText.substr(0, 16) + "...";
                }
                usernameElem.textContent = usernameText;
            }
        })
        .catch(error => {
            console.error('Error:', error);
            redirectToLogin();
        });
    } else {
        redirectToLogin();
    }
}
