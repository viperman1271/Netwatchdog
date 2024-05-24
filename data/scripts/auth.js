function checkTokenAndRedirect() {
    const token = localStorage.getItem('authToken');
    if (token) {
        fetch('/protected', {
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
            console.log('Protected data:', data);
            // Optionally, display the data on the page
            const resultDiv = document.getElementById('result');
            if (resultDiv) {
                resultDiv.textContent = data;
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

document.addEventListener('DOMContentLoaded', function() {
    checkTokenAndRedirect();
});

function redirectToLogin() {
    if(window.location.pathname !== "/login.html") {
        window.location.href = '/login.html';
    }
}