document.addEventListener('DOMContentLoaded', function() {
    const sections = document.querySelectorAll('section');
    const observerOptions = {
        root: null,
        rootMargin: '0px',
        threshold: 0.1
    };

    const observer = new IntersectionObserver((entries, observer) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.classList.add('visible');
            }
        });
    }, observerOptions);

    sections.forEach(section => {
        observer.observe(section);
    });
});

document.addEventListener('DOMContentLoaded', function() {
    // Smooth Scroll Function
    function smoothScroll(target, duration) {
        const targetElement = document.querySelector(target);
        if (!targetElement) return; // Exit if the target element is not found

        const targetPosition = targetElement.offsetTop;
        const startPosition = window.pageYOffset;
        const distance = targetPosition - startPosition;
        let startTime = null;

        function animation(currentTime) {
            if (startTime === null) startTime = currentTime;
            const timeElapsed = currentTime - startTime;
            const run = ease(timeElapsed, startPosition, distance, duration);
            window.scrollTo(0, run);
            if (timeElapsed < duration) requestAnimationFrame(animation);
        }

        function ease(t, b, c, d) {
            t /= d / 2;
            if (t < 1) return c / 2 * t * t + b;
            t--;
            return -c / 2 * (t * (t - 2) - 1) + b;
        }

        requestAnimationFrame(animation);
    }

    // Handle Navigation Clicks
    document.querySelectorAll('a').forEach(anchor => {
        anchor.addEventListener('click', function(e) {
            const target = this.getAttribute('href');

            // Check if href starts with #
            if (target.startsWith('#')) {
                e.preventDefault();
                smoothScroll(target, 1000); // Duration in milliseconds
            }
        });
    });
});

const xValues = ["Crystal", "VSCode", "Visual Studio"];
const yValues = [56, 522, 813];

new Chart("ram-usage-chart", {
  type: "bar",
  data: {
    labels: xValues,
    datasets: [{
      backgroundColor: "#875EDB",
      data: yValues
    }]
  },
  options: {
    plugins: {
        legend: {
            display: false // Set to false to hide the legend
        }
    },
    scales: {
        y: {
            beginAtZero: true
        }
    }
  }
});